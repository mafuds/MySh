#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

//#define tamanhoMaximo 255; NÃO FAZ SENTIDO (VIDE ABAIXO)
const int tamanhoMaximo = 255;

char diretorioConfigurado[255]; // 255 NÃO FAZ SENTIDO 2 (VIDE ACIMA)
char usuarioConfigurado[255];

int houveSinal = 0;

// execvp = executar arquivo
// sigaction = muda a ação que é feita quando recebe um sinal
// getcwd = pega diretório atual
// getenv = get an environment variable
// gethostname = 
// strtok = separa string em tokens (tipo pch = strtok (str," ,.-");)

/*  Gera um processo filho executando um novo programa.
	PROGRAM é o nome do programa a ser executado; ele
	será buscado no path. ARG_LIST é uma lista terminada
	em NULL de strings de caracteres a serem passados
	como a lista de argumentos do programa. Retorna o id
	do processo gerado. */
int spawn (char* program, char** arg_list) {
	pid_t child_pid;
	/* Duplicar este processo. */
	child_pid = fork (); /* Gera um processo filho */
	if (child_pid != 0) 
		/* Este é o processo pai. */
		return child_pid;
	else {
		/* Agora execute PROGRAM, buscando-o no path. */
		execvp (program, arg_list);
		/* A função execvp só retorna se um erro ocorrer. */
		fprintf (stderr, "Falha na execucao do comando '%s'\n", program);
		abort ();
	}
}

void handler (int numSinal) {
	//printf("handler");
	houveSinal = 1;
}

int rodaPrograma(char strteste[50]) {
	char * aux;
	char * arg_list[20];
	int child_status;
	
	aux = strtok(strteste, " ");
	int i = 0;
	while (aux != NULL) {
		//printf ("%s\n", aux);
		arg_list[i] = (char*)malloc (sizeof(char)*strlen(aux));
		strcpy(arg_list[i], aux);
		i++;
		aux = strtok (NULL, " ");
	}
	arg_list[i] = '\0';

	spawn (arg_list[0], arg_list);	
	// Espera acabar a execução do spawn antes de voltar pro main
	wait(&child_status);
	return 0;
}

void mudaDiretorio() {

	int i = 0;
	char usuario[tamanhoMaximo];
	char hospedeiro[tamanhoMaximo];
	char usuarioHosp[tamanhoMaximo];
	char diretorioAtual[tamanhoMaximo];
	char homeUsuario[tamanhoMaximo];
	char novoEndereco[tamanhoMaximo];
	char * token;
	
	getcwd(diretorioAtual, tamanhoMaximo);
	strcpy(homeUsuario, "/home/");
	strcat(homeUsuario, usuario);
	
	getlogin_r(usuario, tamanhoMaximo);
	gethostname(hospedeiro, tamanhoMaximo);
	strcpy(usuarioHosp, usuario);
	strcat(usuarioHosp, "@");
	strcat(usuarioHosp, hospedeiro);
	strcpy(usuarioConfigurado, usuarioHosp);
	
	
	if (strncmp(diretorioAtual, homeUsuario, strlen(homeUsuario))) {
		// precisa substituir por ~
		token = strtok (diretorioAtual, "/");
		
		while (i < 2 && token != NULL) {
			token = strtok(NULL, "/");
			i++;
		}
		
		strcpy(novoEndereco, "~/");
		
		while (token != NULL) {
			strcat(novoEndereco, token);
			token = strtok(NULL, "/");
			if (token != NULL) {
				strcat(novoEndereco, "/");
			}
		}
		strcpy(diretorioConfigurado, novoEndereco);
	}
}

int saida() {
	//int tamanhoMaximo = 255;
	
	char comando[tamanhoMaximo];
	char * token;
	char * argumento;
	struct sigaction sigact;
	
	// para bloquear o uso de ctrl c e ctrl z
	memset(&sigact, 0, sizeof(sigact));
	sigact.sa_handler = &handler;
	// ctrl c
	sigaction(SIGINT, &sigact, NULL);
	// ctrl z
	sigaction(SIGTSTP, &sigact, NULL);
	
	//printf("sinal: %d\n",houveSinal);
	mudaDiretorio();
	
	// laço para receber comandos até receber o comando exit
	do {
		printf("[MySh] %s:%s$  ", usuarioConfigurado, diretorioConfigurado);
		fgets(comando, tamanhoMaximo, stdin);
		
		if (houveSinal) {
			// passa a variavel de volta pra 0
			houveSinal = 0;
			fflush(stdin);
			// se o comando for vazio, ele nem entra no if (não tenta executar o ctrl c ou ctrl z)
			strcpy(comando, "");
			printf("\n");
		}
		
		// configura o shell pra sair do programa no ctrl D
		if (feof(stdin)) {
			printf("\n");
			exit(0);
		}
			
		if (strlen(comando) > 0) {
	
			
			// Troca o \n no final do comando por \0 -> se não fizer isso, 
			// dá segmentation fault
			if (comando[strlen(comando)-1] == '\n') {
				comando[strlen(comando)-1] = '\0'; 
			}
			
			if(strncmp(comando,"cd",2) == 0) { // o comando É cd
				if(strlen(comando) == 2) { // se não tiver argumentos, abre a pasta 'home'
					chdir("/home");
				}
				else { // senão, vai para o caminho passado
					argumento = strtok(comando," ");
					
					if(chdir((char *)argumento + 3) == -1) {
						fprintf(stderr,"Caminho não encontrado\n");
					}
				}
				
				mudaDiretorio();
				
			} else if (strcmp(comando, "exit") != 0) {
				rodaPrograma(comando);
			}
		}

		
	} while (strcmp(comando, "exit") != 0);
	
	return 0;
}

int main() {
    saida();
	return 0;
}
