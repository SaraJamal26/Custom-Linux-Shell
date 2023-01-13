//the command file are in bashscript and must be saved in /bin/ directory. Also, it must be given permission.
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

#define MAXLENGTH 1000 // max length of command
#define MAXCOMMAND 100 // max number of commands to be supported
#define GREEN_COLOR "\x1b[32m"
#define BLUE_COLOR "\x1b[34m"
#define CYAN_COLOR "\x1b[36m"
#define RESET_COLOR "\x1b[0m"
#define UNDERLINE "\e[4m"
#define REMOVE_UNDERLINE "\e[24m"
#define BOLD "\e[1m"
#define BOLDOFF "\e[0m"
#define POINT "\u2022"
#define clear() printf("\033[H\033[J")

//All functions prototype
void Shell_Message();
int Command_Input(char* command);
void Current_Path();
void Execution(char** parsed);
void Piped_Execution(char** parsed, char** parsedpipe);
void Help_Function();
int Custom_Commands(char** parsed);
int Parses_Pipe(char* Command, char** string_piped);
void Parse_Space(char* Command, char** parsed);
int Execute_Cmd(char* Command, char** parsed, char** parsedpipe);

int main(){
    char command[MAXLENGTH], *parsedArgs[MAXCOMMAND];
    char* parsedArgsPiped[MAXCOMMAND];
    int cmdFlag = 0;
    Shell_Message();
    while (1) {
        Current_Path();
        // take input
        if (Command_Input(command))
            continue;
        
        cmdFlag = Execute_Cmd(command,parsedArgs, parsedArgsPiped);
        //returns 0 if null or customized command,

        // execute
        if (cmdFlag == 1)      //if simple command
            Execution(parsedArgs);

        if (cmdFlag == 2)      //if piped command
            Piped_Execution(parsedArgs, parsedArgsPiped);
    }
    return 0;
}
void Shell_Message(){
    clear();
    printf("\n\n\n\n--------------------------------------------------------");
    printf(BOLD CYAN_COLOR "\n\n\n\t****[ SHELL IMPLEMENTATION ]****" BOLDOFF);
    printf("\n\n    Project by 19K-1256, 19K-0207 & 19K-1510");
    printf("\n\n--------------------------------------------------------");
    printf("\n\n--------------------------------------------------------"RESET_COLOR);
    sleep(2);
    clear();
    char* username = getenv("USER");
    printf("\n\n\nCurrent user: @%s", username);
    printf("\n");
    sleep(1);
    printf(CYAN_COLOR"\n----------Enter command or exit to close this shell---------- \n");
    clear();
}

int Command_Input(char* command){
    char* input;
    input = readline("$ ");   //take input

    if (strlen(input) != 0) {
        add_history(input);
        strcpy(command, input);
        return 0;
    }
    else
    {
        return 1;
    }
}

//shows current path and username
void Current_Path(){
    char path[1024];
    getcwd(path, sizeof(path));
    printf(BOLD BLUE_COLOR UNDERLINE"\n%s\n" RESET_COLOR REMOVE_UNDERLINE BOLDOFF, path);
    char* username = getenv("USER");
    printf(BOLD GREEN_COLOR UNDERLINE"\n@%s"REMOVE_UNDERLINE" ---> "RESET_COLOR BOLDOFF, username );
}

// for executing simple commands
void Execution(char** parsed){
    pid_t pid = fork();
    if (pid == 0) {   //if child..
        if (execvp(parsed[0], parsed) < 0)   //if execution fails
            printf("\nSorry! Could not execute command..");
        
        exit(0);
    } 
    else if(pid == -1) {   //if forking fails
        printf("\nFailed forking child..");
        return;
    }
    else {        //else if parent, wait for child to terminate
        wait(NULL);
        return;
    }
}

//for executing piped commands
void Piped_Execution(char** parsed, char** parsedpipe){
    int pipefd[2];
    pid_t pipe1, pipe2;
      //0 = READ_END , 1 = WRITE_END
    if (pipe(pipefd) < 0) {
        printf("\nSorry! Pipe initialization unsuccessful");
        return;
    }
    pipe1 = fork();
    if (pipe1 < 0) {   //forking fails
        printf("\nForking unsuccessful!");
        return;
    }

    if (pipe1 == 0) {  //if child 1..
        // It only needs to write at the write end
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        if (execvp(parsed[0], parsed) < 0) {
            printf("\nCould not execute command 1..");
            exit(0);
        }
    } 
	else {    //else if parent executing
        pipe2 = fork();

        if (pipe2 < 0) {
	    printf("\nForking unsuccessful!");
            return;
        }

        
        if (pipe2 == 0)  {  //if child 2...
	// It only needs to read at the read end	
            close(pipefd[1]);
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);
            if (execvp(parsedpipe[0], parsedpipe) < 0) {
                printf("\nCould not execute command 2..");
                exit(0);
            }
        } 
		else { //else if parent, wait for 2 children to terminate
            wait(NULL);
            wait(NULL);
        }
    }
}

// Shows function of each customized command
void Help_Function(){
    printf("\nCommands available in this shell:\n\n"
	POINT "add: Adds given number of values\n"
	POINT "cd [path]: Changes current working directory to the specified directory\n"
	POINT "compile [type] [filename]: compiles c files of different types\n"
        "type-- s=simple c files\n       p=c file with pthread\n       o=c file with openmp"
	POINT "divide: Divides a given value with another\n"
	POINT "exit: Exits shell\n"
	POINT "ext [filename]: return type of file by using its extension\n"
	POINT "factorial: Calculates factorial of a given number\n"
	POINT "lc: lists files and folder with names starting from lowercase\n"
	POINT "meminfo: Displays memory information\n"
	POINT "multiply: Multiplies given number of values\n"
	POINT "osinfo: Displays information about current operating system\n"
	POINT "run [webpage name]: run the website on firefox\n"
	POINT "showdir: lists only directories in the current folder\n"
        POINT "showip: Shows current system IP address\n"
	POINT "subtract: Subtracts a given value from another\n"
	POINT "modulo: Gives the remainder of a certain division\n"
	POINT "topram: Displays top 5 programs consuming the most RAM\n"
	POINT "upc: lists files and folder with names starting from uppercase\n"
	POINT "update: installs available update & removes unnecessary packages\n"
	POINT "compare: compares two files, numbers or strings at a time\n"
	POINT "userinfo: Shows user information\n"	
        POINT "sc [filename]: copy sorted contents of given filename to copy[filename]\n"
	POINT "search [filename] [word]: returns the position of first occurence of word in file\n"
    	POINT "viewperm [filename]: Displays permissions in words of given file\n"
	POINT "wordfreq [filename] [word]: Displays frequency and occurence of a given word in a given file\n"
    	POINT "other general linux commands are also available\n");
    return;
}

int Custom_Commands(char** parsed){
    int custom_num = 27, cmdNo = 0, fd,n,arr[16],sum=0,prod=1,fact=1;
    int result,first,second,dividend,divisor,i;
    char* Commands[custom_num], *username, *base;
    char path[1024], cmd[1024] = {0};
    unsigned char ip_address[16];
    struct ifreq ifr;
    pid_t pid;

//all command files are stored in /bin
    Commands[0] = "exit";
    Commands[1] = "topram";
    Commands[2] = "help";
    Commands[3] = "hello";
    Commands[4] = "showip";
    Commands[5] = "add";
    Commands[6] = "multiply";
    Commands[7] = "subtract";
    Commands[8] = "divide";
    Commands[9] = "upc";
    Commands[10] = "lc";
    Commands[11] = "showdir";
    Commands[12] = "run";
    Commands[13] = "meminfo";
    Commands[14] = "update";
    Commands[15] = "modulo";
    Commands[16] = "userinfo";
    Commands[17] = "cd";
    Commands[18] = "osinfo";
    Commands[19] = "factorial";
    Commands[20] = "sc";
    Commands[21] = "compile";
    Commands[22] = "viewperm";
    Commands[23] = "ext";
    Commands[24] = "compare";
    Commands[25] = "search";
    Commands[26] = "wordfreq";

    for (int i = 0 ; i < custom_num ; i++) 
	{
        if (strcmp(parsed[0], Commands[i]) == 0) 
		{
            cmdNo = i + 1;
            break;
        }
    }

    switch (cmdNo) 
	{
    case 1:
	printf("\nExiting Shell...\n");
	sleep(1);
	printf("\nBye!\n\n");
    exit(0);
	break;
    case 2:
        pid = fork();
       		if(pid==0)
			{
          	execl("/bin/cmd","cmd",(char*)0);
		    return 1;
		    } 		
    		else if(pid == -1) 
			{
        	   printf("\nFailed forking child..");
        	   return 1;
    		}
    		else 
			{
        	   wait(NULL);
        	   return 1;
    		}
    		return 1;
    case 3:
        Help_Function();
        return 1;
    case 4:
        username = getenv("USER");
        printf("\nHello %s.\n\n%s, please use 'help' to get more information about this shell..\n",username, username);
        return 1;
    case 5:
        fd = socket(AF_INET, SOCK_DGRAM, 0);
    	ifr.ifr_addr.sa_family = AF_INET;
    	memcpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);
    	ioctl(fd, SIOCGIFADDR, &ifr);
    	close(fd);     
    	strcpy(ip_address,inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
    	printf("System IP Address: %s\n",ip_address);
		return 1;
		
    case 6:
		printf("\nNumber of values to add: ");
		scanf("%d",&n);
		printf("\nEnter values: \n");
		for(int i = 0 ; i < n ; i++)
		{
			scanf("%d",&arr[i]);
  			sum = sum + arr[i];
		}
		printf("\nSum: %d\n",sum);
		return 1;

    case 7:
		printf("\nNumber of values to multiply: ");
		scanf("%d",&n);
		printf("\nEnter values: \n");
		for(int i=0;i<n;i++)
		{
			scanf("%d",&arr[i]);
  			prod = prod * arr[i];
		}
		printf("\nProduct: %d\n",prod);
		return 1;

    case 8:
		printf("\nEnter first operand: ");
		scanf("%d",&first);
		printf("\nEnter second operand: ");
		scanf("%d",&second);
		result = first - second;
		printf("\nDifference: %d",result);
		return 1;

    case 9:
		printf("\nEnter first operand: ");
		scanf("%d",&first);
		printf("\nEnter second operand: ");
		scanf("%d",&second);
		result = first / second;
		printf("\nQuotient: %d",result);
		return 1;
 	
    case 10:
		pid = fork();
       		if(pid==0)
			{
		   execl("/bin/upc","upc",(char*)0);
		   return 1;
		    } 		
    		else if(pid == -1) 
			{
        	   printf("\nFailed forking child..");
        	   return 1;
    		}
    		else 
			{
        	   wait(NULL);
        	   return 1;
    		}
		return 1;

    case 11:
		pid = fork();
       		if(pid==0)
		    {
		    execl("/bin/lc","lc",(char*)0);
		    return 1;
		    }  		
    		else if(pid == -1) 
			{
        	   printf("\nFailed forking child..");
        	   return 1;
    		}
    		else 
			{
        	   wait(NULL);
        	   return 1;
    		}
		return 1;

    case 12:
		pid = fork();
       		if(pid==0)
			{
          	execl("/bin/showdir","showdir",(char*)0);
		    return 1;
		    } 		
    		else if(pid == -1) 
			{
        	   printf("\nFailed forking child..");
        	   return 1;
    		}
    		else 
			{
        	   wait(NULL);
        	   return 1;
    		}
    		return 1;

    case 13:
		base = "/bin/run "; 
  		sprintf(cmd, "%s", base);
		sprintf(cmd, "%s%s ", cmd, parsed[1]);
		printf("%s\n",cmd);
  		system(cmd);
		return 1;
     
    case 14:
		pid = fork();
       		if(pid==0)
			{
          	execl("/bin/mem","mem",(char*)0);
		    return 1;
		    } 		
    		else if(pid == -1) 
			{
        	   printf("\nFailed forking child..");
        	   return 1;
    		}
    		else 
			{
        	   wait(NULL);
        	   return 1;
    		}
    		return 1;

    case 15:
		pid = fork();
       		if(pid==0)
			{
          	execl("/bin/update","update",(char*)0);
		    return 1;
		    } 		
    		else if(pid == -1) 
			{
        	   printf("\nFailed forking child..");
        	   return 1;
    		}
    		else 
			{
        	   wait(NULL);
        	   return 1;
    		}
    		return 1;

		
    case 16:
		printf("\nEnter dividend: ");
		scanf("%d",&dividend);
		printf("\nEnter divisor: ");
		scanf("%d",&divisor);
		result = dividend % divisor;
		printf("\nModulo: %d",result);
		return 1;
    
    case 17:
		pid = fork();
       		if(pid==0)
			{
          	execl("/bin/userinfo","userinfo",(char*)0);
		    return 1;
		    } 		
    		else if(pid == -1) 
			{
        	   printf("\nFailed forking child..");
        	   return 1;
    		}
    		else 
			{
        	   wait(NULL);
        	   return 1;
    		}
    		return 1;

    case 18:
		if( chdir(parsed[1]) != 0 )
		perror(parsed[1]);
		return 1;

    case 19:
		pid = fork();
       		if(pid==0)
			{
          	execl("/bin/osinfo","osinfo",(char*)0);
		    return 1;
		    } 		
    		else if(pid == -1) 
			{
        	   printf("\nFailed forking child..");
        	   return 1;
    		}
    		else 
			{
        	   wait(NULL);
        	   return 1;
    		}
    		return 1;

   case 20:
		printf("\nEnter a number: ");
		scanf("%d",&n);
		for(i = 1; i < n; i++)
		fact = fact*i;
		printf("Factorial of %d = %d\n",n,fact);
		return 1;
		
    case 21:
		base = "/bin/sc "; 
  		sprintf(cmd, "%s", base);
		sprintf(cmd, "%s%s ", cmd, parsed[1]);
  		system(cmd);
		return 1;

   case 22:
		base = "/bin/compile "; 
  		sprintf(cmd, "%s", base);
		sprintf(cmd, "%s%s ", cmd, parsed[1]);
		sprintf(cmd, "%s%s ", cmd, parsed[2]);
  		system(cmd);
		return 1;
   case 23:
		base = "/bin/viewperm "; 
  		sprintf(cmd, "%s", base);
		sprintf(cmd, "%s%s ", cmd, parsed[1]);
  		system(cmd);
		return 1;
   case 24:
		base = "/bin/ext "; 
  		sprintf(cmd, "%s", base);
		sprintf(cmd, "%s%s ", cmd, parsed[1]);
  		system(cmd);
		return 1;
   case 25:
		pid = fork();
       		if(pid==0)
			{
          	execl("/bin/compare","compare",(char*)0);
		    return 1;
		    } 		
    		else if(pid == -1) 
			{
        	   printf("\nFailed forking child..");
        	   return 1;
    		}
    		else 
			{
        	   wait(NULL);
        	   return 1;
    		}
    		return 1;
    case 26:
		base = "/bin/search "; 
  		sprintf(cmd, "%s", base);
		sprintf(cmd, "%s%s ", cmd, parsed[1]);
		sprintf(cmd, "%s%s ", cmd, parsed[2]);
  		system(cmd);
		return 1;

    case 27:
		base = "/bin/wordfreq "; 
  		sprintf(cmd, "%s", base);
		sprintf(cmd, "%s%s ", cmd, parsed[1]);
		sprintf(cmd, "%s%s ", cmd, parsed[2]);
  		system(cmd);
		return 1;

    default:
        break;
    }

    return 0;
}


int Parsed_Pipe(char* Command, char** string_piped){
    for (int i = 0; i < 2; i++) {
        string_piped[i] = strsep(&Command, "|");  //separates command if pipes found
        if (string_piped[i] == NULL){
        	break;
		}    
    }

    if (string_piped[1] == NULL)
        return 0;                // returns 0 if no pipes found
    else 
	{
        return 1;
    }
}

void Parse_Space(char* Command, char** parsed){
    for (int i = 0; i < MAXCOMMAND; i++) 
	{
        parsed[i] = strsep(&Command, " ");    //separate command if spaces found

        if (parsed[i] == NULL)
            break;
        if (strlen(parsed[i]) == 0)
            i--;
    }
}

int Execute_Cmd(char* Command, char** parsed, char** parsedpipe){
    char* string_piped[2];
    int piped = 0;
    piped = Parsed_Pipe(Command, string_piped);   //to separate commands by pipes, 
						  //returns 1 if pipe found
    if(piped){
        Parse_Space(string_piped[0], parsed);     //parses first part of piped command
        Parse_Space(string_piped[1], parsedpipe); //parses second part of piped command
    } 
    else
	{
        Parse_Space(Command, parsed);             //parses simple commands
    }
    if(Custom_Commands(parsed))			  //executes parsed commands
        return 0;
    else
        return 1 + piped;
}
