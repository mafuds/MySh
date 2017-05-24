/**
*
* Trabalho 1 - Sistemas Operacionais 2
* Grupo: Artur Mafud, Mariana Fantini, Mariana Marques Pacheco
* 
**/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

const int tamanhoMaximo = 255;

char diretorioConfigurado[255];
char usuarioConfigurado[255];

int houveSinal = 0;

/** 
* Função "spawn" executa o que foi dado como argumento em "program" e 
* passa array dado por "arg_list" como argumento
**/
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

/**
* Função "handler" é usada para controle de ctrl c e ctrl z
**/
void handler (int numSinal) {
	houveSinal = 1;
}

/**
* Função "rodaPrograma" trata e executa o comando entrado pelo usuário
**/
int rodaPrograma(char strComando[255]) {
	char * aux;
	char * arg_list[20];
	int child_status;

	aux = strtok(strComando, " ");
	int i = 0;
	while (aux != NULL) {
		arg_list[i] = (char*)malloc (sizeof(char)*strlen(aux));
		strcpy(arg_list[i], aux);
		i++;
		aux = strtok (NULL, " ");
	}
	// termina arg_list com símbolo de final de string
	arg_list[i] = '\0';

	// arg_list[0] é o comando a ser executado e arg_list é a lista de argumentos que ele recebe
	spawn (arg_list[0], arg_list);

	// Espera acabar a execução do spawn antes de voltar pro main
	wait(&child_status);
	return 0;
}

/**
* Função "mudaDiretorio" trata como é escrito o diretório na shell.
* Deve ser no formato [MySh] nome-de-usuario@hospedeiro:diretorio-atual$
* e o diretório no formato "/home/nome-do-usuario" deve ser substituído por ~
**/
void mudaDiretorio() {

	int i = 0;
	char usuario[tamanhoMaximo];
	char hospedeiro[tamanhoMaximo];
	char usuarioHosp[tamanhoMaximo];
	char diretorioAtual[tamanhoMaximo];
	char homeUsuario[tamanhoMaximo];
	char novoEndereco[tamanhoMaximo];
	char * token;

    // pegamos o usuario atual e o nome de hospedeiro
	getlogin_r(usuario, tamanhoMaximo);
    gethostname(hospedeiro, tamanhoMaximo);
	
	// homeUsuario valerá /home/nome-do-usuario
	getcwd(diretorioAtual, tamanhoMaximo);
	strcpy(homeUsuario, "/home/");
	strcat(homeUsuario, usuario);

	// usuarioHosp valerá nome-de-usuario@hospedeiro
	strcpy(usuarioHosp, usuario);
	strcat(usuarioHosp, "@");
	strcat(usuarioHosp, hospedeiro);
	strcpy(usuarioConfigurado, usuarioHosp);

	// se o diretorio atual da shell começar com /home/nome-do-usuario, mudamos isso por ~/
	if (strncmp(diretorioAtual, homeUsuario, strlen(homeUsuario)) == 0) {
		// percorremos a parte da string "/home/nome-do-usuario" 
		token = strtok (diretorioAtual, "/");
		while (i < 2 && token != NULL) {
			token = strtok(NULL, "/");
			i++;
		}
		
		// iniciamos o novo endereço com o ~/
		strcpy(novoEndereco, "~/");

		// agora pegamos o resto do caminho do diretorio atual (depois da parte "/home/nome-do-usuario")
		while (token != NULL) {
			strcat(novoEndereco, token);
			token = strtok(NULL, "/");
			if (token != NULL) {
				strcat(novoEndereco, "/");
			}
		}
		strcpy(diretorioConfigurado, novoEndereco);
	} else {
		strcpy(diretorioConfigurado, "");
		strcpy(novoEndereco, "/");
		token = strtok (diretorioAtual, "/");
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

/**
* Função "rodaPipe" trata os comandos que contenham pipe
**/
void rodaPipe(char comando[tamanhoMaximo]) {

	int pipefds[2];
	int ultimo;
	int i = 0, j = 0;
	int status;
	int numComandos = 0;
	char * comandos;
	char * argumentos;
	pid_t pid;
	// arg_list receberá a lista de comandos digitada pelo usuario e serão executados
	char * arg_list[20];
	// arg_list2 recebe a lista de tokens para executar cada comando em arg_list
	// obs: a variavel será limpa a cada iteração para não ter lixo de memória 
	// 		quando for executar novo comando em arg_list2
	char * arg_list2[20];

	comandos = strtok(comando,"|");
	while (comandos != NULL) {
		arg_list[numComandos] = (char*)malloc (sizeof(char)*strlen(comandos));
		strcpy(arg_list[numComandos], comandos);
		numComandos++;
		comandos = strtok(NULL, "|");
	}
	arg_list[numComandos] = '\0';

	for (i = 0; i < numComandos-1; i++) {
		
		// cria o pipe
		pipe(pipefds);

		pid = fork();
		if (pid == (pid_t) 0) {
			// Processo filho
			if (i > 0) {
				// caso não seja o primeiro comando, lê o output do ultimo pipe
				dup2(ultimo, 0);
			}
			// escreve no pipe atual
			dup2(pipefds[1], 1);

			// limpamos a string arg_list2
			arg_list2[0] = "\0";
			j = 0;
			argumentos = strtok(arg_list[i], " ");
			while (argumentos != NULL) {
				arg_list2[j] = (char*)malloc (sizeof(char)*strlen(argumentos));
				strcpy(arg_list2[j], argumentos); // coloca o token em arg_list2
				j++;
				argumentos = strtok(NULL, " ");
			}
			// termina arg_list2 com simbolo de final de string
			arg_list2[j] = '\0';

			// executa o comando em arg_list2
			execvp(arg_list2[0], arg_list2);

		} else {
			// Processo pai
			wait(&status); // espera processo filho acabar
		}
		close(pipefds[1]);

		if(i > 0) {
			close(ultimo);
		}

		ultimo = pipefds[0];
	}

	pid = fork();
	if (pid == (pid_t) 0) {
		if (i > 0) {
			// caso não seja o primeiro comando, lê o output do ultimo pipe
			dup2(ultimo, 0);
		}

		// limpamos a string arg_list2
		arg_list2[0] = "\0";
		j = 0;
		argumentos = strtok(arg_list[i], " ");
		while (argumentos != NULL) {
			arg_list2[j] = (char*)malloc (sizeof(char)*strlen(argumentos));
			strcpy(arg_list2[j], argumentos);
			j++;
			argumentos = strtok(NULL, " ");
		}
		// termina arg_list2 com simbolo de final de string
		arg_list2[j] = '\0';

		// executa o comando que está em arg_list2
		execvp(arg_list2[0], arg_list2);
	} else {
		wait(&status); //espera o processo filho acabar para impedir processo zumbi
	}
}

/**
* Funçao "terminal" carrega o nosso shell
**/
int terminal() {
	//int tamanhoMaximo = 255;
	char usuario[tamanhoMaximo];
	char comando[tamanhoMaximo];
	char * argumento;
	struct sigaction sigact;

	// para bloquear o uso de ctrl c e ctrl z
	memset(&sigact, 0, sizeof(sigact));
	sigact.sa_handler = &handler;
	// ctrl c
	sigaction(SIGINT, &sigact, NULL);
	// ctrl z
	sigaction(SIGTSTP, &sigact, NULL);

	mudaDiretorio();

	// laço para receber comandos até receber o comando exit
	do {
		printf("[MySh] %s:%s$  ", usuarioConfigurado, diretorioConfigurado);
		fgets(comando, tamanhoMaximo, stdin);

		if (houveSinal) {
			// passa a variavel de volta pra 0
			houveSinal = 0;
			fflush(stdin);
			// se o comando for vazio, não entra no if e não haverá tentativa de executar o ctrl c ou ctrl z
			strcpy(comando, "");
			printf("\n");
		}

		// configura o shell pra sair do programa no ctrl D
		if (feof(stdin)) {
			printf("\n");
			exit(0);
		}

		if (strlen(comando) > 0) {
			// Configura a string: troca o \n no final do comando por \0 
			// obs: se não fizer isso, dá segmentation fault
			if (comando[strlen(comando)-1] == '\n') {
				comando[strlen(comando)-1] = '\0';
			}

			// testa se o comando digitado começa com "cd"
			if(strncmp(comando,"cd",2) == 0) {
				// se não tiver argumentos, vai pra pasta 'home'
				if(strlen(comando) == 2) {
					chdir("/home");
				} else { // se tiver argumentos, vai para o caminho passado
					argumento = strtok(comando," ");
					if (strcmp((char *)argumento + 3, "~") == 0) {
						// se for cd ~, vai pra home/usuario
						getlogin_r(usuario, tamanhoMaximo);
						chdir("/home");
						chdir(usuario);
					} else if(chdir((char *)argumento + 3) == -1) {
					// se não for ~, vai pro caminho
					// se ele nao conseguir ir pra pasta (ou seja, chdir retornou -1), apresenta msg de erro
						fprintf(stderr,"Caminho não encontrado\n");
					}
				}
				
				// como mudamos de diretório, a função que determina o nome de diretório é chamada
				mudaDiretorio();

			} 
			// se o comando não é cd e nem exit
			else if (strcmp(comando, "exit") != 0) {
				// testamos se o comando tem pipe
				if (strchr(comando, '|') != NULL) {
					rodaPipe(comando);
				} else {
				// se o comando não tem pipe, executa o comando normalmente
					rodaPrograma(comando);
				}
			}
		}
	} while (strcmp(comando, "exit") != 0);

	return 0;
}

/**
* O main do programa apenas chama a shell
**/
int main() {
    terminal();
	return 0;
}
