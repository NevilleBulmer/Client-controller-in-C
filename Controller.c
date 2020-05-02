/* Main system includes */
#include <stdio.h>      // printf
#include <string.h>     // memset
#include <stdlib.h>     // exit(0);
#include <arpa/inet.h>  // ports and addresses
#include <sys/socket.h> // sockets
#include <pthread.h>    // threads

/* Extra functionality includes */
#include <termios.h>    // terminal I/O
#include <unistd.h>     // primarily for sleep
#include <fcntl.h>      // file control options
#include <ctype.h>      // testing and mapping characters
#include <time.h>       // time

// The ip on which to connect to.
#define SERVERIP "192.168.99.1"
// Max length of buffer.
#define BUFFERLENGTH 1024
// The port on which to send/recieve data.
#define LANDERSERVERPORT 65200
// The port on which to send data.
#define DASHSERVERPORT 65250

// Struct for the lander socket address in
struct sockaddr_in socketInLander;
// Struct for the dashboard socket address in
struct sockaddr_in socketInDash;

// Create ints for use throughout the class.
int s, 
    stringLengthLander=sizeof(socketInLander), 
    stringLengthDash=sizeof(socketInDash);

// Thread for the lander, dashboard, keystroke detection and the data logging component.
pthread_t lander_communication_thread,
          dashboard_communication_thread, 
          keystroke_detection, 
          data_logging;

// Die (exit) if a problem occurs.
void die(char *s)
{
    // Grab the error that occured, I.e. s is the error.
    perror(s);
    // Exit the program.
    exit(1);
}

// Set the mode I.e. used to et the mode for the while loop in detecUserInput.
void initialize_mode(int needed_key)
{
    // Instantiate two termios structs, old, new.
	static struct termios old, new;

    // Check if it is not needed_key.
	if (!needed_key) 
    {
        // termios functions.
		tcsetattr(STDIN_FILENO, TCSANOW, &old);
        // Return.
		return;
	}
 
	tcgetattr(STDIN_FILENO, &old);

    // Instantiate/set new equal to old.
	new = old;

	new.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &new);
}
 
// Used to detec which key is being pressed, which is then passed detectUserInput.
int get_keystroke()
{
    // Instantiate int keystroke to 0.
	int keystroke = 0;

    // Instantiate a timeval strcut timeValue.
	struct timeval timeValue;
    // Instantie an fd variable to fs.
	fd_set fs;
	timeValue.tv_usec = timeValue.tv_sec = 0;
 
    // Clears fs.
	FD_ZERO(&fs);

    // Sets fs to stdin.
	FD_SET(STDIN_FILENO, &fs);
    
	select(STDIN_FILENO + 1, &fs, 0, 0, &timeValue);
 
	if (FD_ISSET(STDIN_FILENO, &fs)) 
    {
        // Set keystroke equal to getchar, in other words keystroke is now equal to the key which was pressed.
		keystroke = getchar();

        // Using the initialize_mode method set mode to zero.
		initialize_mode(0);
	}

    // Return the keystroke variable to be used in the detectUserInput method.
	return keystroke;
}

// Connect to the lnader server.
void *connectToLander()
{
    // Create a client socket.
    if((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)       
    {
        // Die.
        die("socket");
    }

    // Connect to the lander.
    memset((char *) & socketInLander, 0, sizeof(socketInLander));
    // Sets socket in sin_familly to AFI_NET.
    socketInLander.sin_family = AF_INET;
    // Sets socket in sin_port to landers server port.
    socketInLander.sin_port = htons(LANDERSERVERPORT);

    // Create datagram for the lander with server IP and port.
    if(inet_aton(SERVERIP , &socketInLander.sin_addr) == 0)            
    {
        // fprintf any given error.
        fprintf(stderr, "inet_aton() failed\n");
        // exit.
        exit(1);
    } else {
        // printf connection successful with the port.
        printf("Attempting connection..... Connection to lander established, port %d\n", LANDERSERVERPORT);
    }

    return 0;
}

// Connect to the Dash server.
static void *connectToDash()
{                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        
    // Create a client socket.
    if((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)       
    {
        // Die.
        die("socket");
    }

    memset((char *) & socketInDash, 0, sizeof(socketInDash));
    // Sets socket in sin_familly to AFI_NET.
    socketInDash.sin_family = AF_INET;
    // Sets socket in sin_port to dash server port.
    socketInDash.sin_port = htons(DASHSERVERPORT);

    // Create datagram for dash with server IP and port.
    if(inet_aton(SERVERIP , &socketInDash.sin_addr) == 0)            
    {
        // fprintf any given error.
        fprintf(stderr, "inet_aton() failed\n");
        // exit.
        exit(1);
    } else {
        // printf connection successful with the port.
        printf("Attempting connection..... Connection to dash established, port %d\n", DASHSERVERPORT);
        
        // Opening menu, so the user knows what to use to control the lander.
        // I decided to make it look retro as an homage to the classic terminal based games from the past.
        printf("\n");

        printf("          ###############################################\n");
        printf("          #                                             #\n");
        printf("          #          Welcome to Lunar Lander            #\n");
        printf("          #                                             #\n");
        printf("          #         A game to test your might           #\n");
        printf("          #                                             #\n");
        printf("          ###############################################\n");

        printf("\n");

        printf("                                |                       \n");
        printf("                               |_|                      \n");
        printf("                              | _ |                     \n"); 
        printf("                             |.o '.|                    \n");
        printf("                             |'._.'|                    \n");
        printf("                             |     |                    \n");
        printf("                           .'|  |  |'.                  \n");
        printf("                          |  |  |  |  |                 \n");
        printf("                          |.-'--|--'-.|                 \n");

        printf("\n");

        printf("          ###############################################\n");
        printf("          #                                             #\n");
        printf("          #          -To control the lander-            #\n");
        printf("          #                                             #\n");
        printf("          #                  Engines                    #\n");
        printf("          #                                             #\n");
        printf("          #               Key 0 = stop                  #\n");
        printf("          #       keys 1-9 = pecentages of thrust       #\n");
        printf("          #                                             #\n");
        printf("          #            Key A = roll left                #\n");
        printf("          #            Key D = roll right               #\n");
        printf("          #            Key S = stop roll                #\n");
        printf("          #                                             #\n");
        printf("          ###############################################\n");

        printf("\n");
    }

    return 0;
}

// Recieve message
void *logInformation(void *arg)
{
    // Method auto creates the file if it is not found.
    // Used for loggin the information passed around while running normal operations
    // Instantiate a pointer char and set it equal to the argument passed to the method.
    char *informationToWrite = arg;

    // Instantie a pointer file variable.
    FILE *fileToOpen;

    // Use the file variable and set it equal to fopen to opena  given file, 
    // a+ sets it to append and not ovewrite any existing information.
    fileToOpen = fopen("Test.txt", "a+");

        // Check if the file which gets opened if equal to null,
        // if it is then throw an error and exit.
        if(fileToOpen == NULL)
        {
            // Print an error if one occurs.
            printf("Error!");
            // Exit.
            exit(1);             
        }

        strcat(informationToWrite, "\n");

        // fprintf to the information to the opened file.
        fprintf(fileToOpen, "%s", informationToWrite);

        // fclose the file once writting is complete.
        fclose(fileToOpen);
    
    // return 0, all is fine.
    return 0;
}

// User input
void *detecUserInput()
{
    // Used for detecting the users input I.e. A, S, D, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0

    // S, 115, is used to decrease the main engine
    // A, 97, is used to roll left
    // D, 100, is used to roll right

    // 1, 49, is used to increase/decrease to 10%
    // 2, 50, is used to increase/decrease to 20%
    // 3, 51, is used to increase/decrease to 30%
    // 4, 52, is used to increase/decrease to 40%
    // 5, 53, is used to increase/decrease to 50%
    // 6, 54, is used to increase/decrease to 60%
    // 7, 55, is used to increase/decrease to 70%
    // 8, 56, is used to increase/decrease to 80%
    // 9, 57, is used to increase/decrease to 90%
    // 0, 48, is used to decrease to 0% AKA turn engines off
   
    // 
    int keystroke;

    // Turn off the engines.
    char increaseEngineToZeroCommand[BUFFERLENGTH] = "command\nmain-engine: 0\n";
    // Command to set the engines to 10 percent.
    char increaseEngineToTenCommand[BUFFERLENGTH] = "command\nmain-engine: 10\n";
    // Command to set the engines to 20 percent.
    char increaseEngineToTwentyCommand[BUFFERLENGTH] = "command\nmain-engine: 20\n";
    // Command to set the engines to 30 percent.
    char increaseEngineToThirtyCommand[BUFFERLENGTH] = "command\nmain-engine: 30\n";
    // Command to set the engines to 40 percent.
    char increaseEngineToFortyCommand[BUFFERLENGTH] = "command\nmain-engine: 40\n";
    // Command to set the engines to 50 percent.
    char increaseEngineToFiftyCommand[BUFFERLENGTH] = "command\nmain-engine: 50\n";
    // Command to set the engines to 60 percent.
    char increaseEngineToSixtyCommand[BUFFERLENGTH] = "command\nmain-engine: 60\n";
    // Command to set the engines to 70 percent.
    char increaseEngineToSeventyCommand[BUFFERLENGTH] = "command\nmain-engine: 70\n";
    // Command to set the engines to 80 percent.
    char increaseEngineToEightyCommand[BUFFERLENGTH] = "command\nmain-engine: 80\n";
    // Command to set the engines to 90 percent.
    char increaseEngineToNinetyCommand[BUFFERLENGTH] = "command\nmain-engine: 90\n";

    // Stop the lander from rolling, set roll to 0.0.
    char rollStopCommand[BUFFERLENGTH] = "command\nrcs-roll: 0.0\n";
    // Roll the lander right in the given space by 0.5.
    char rollRightCommand[BUFFERLENGTH] = "command\nrcs-roll: +0.5\n";
    // Roll the lander left in the given space by 0.5.
    char rollLeftCommand[BUFFERLENGTH] = "command\nrcs-roll: -0.5\n";

    // while 1, all is good.
	while(1) 
    {
        // set the mode using the method initialize_mode to 1.
        initialize_mode(1);
        // whilekeystroke is not equal to get_keystroke, in other words keystroke is not active,
        // make it sleep, be inactive.
		while (!(keystroke = get_keystroke())) usleep(10000);

        // switch, case to detect and act accordingly for each given keystroke from the user.
        switch(keystroke) 
        {
            // case for S, stop the landers roll.
            case 115:
                    sendto(s, rollStopCommand, strlen(rollStopCommand), 0, (struct sockaddr *) &socketInLander, stringLengthLander);
                    // Send the information about the command pressed to the loggin method.
                    logInformation(rollStopCommand);
                break; 
            // case for A, roll the lander left by 0.5.
            case 97:
                    sendto(s, rollLeftCommand, strlen(rollLeftCommand), 0, (struct sockaddr *) &socketInLander, stringLengthLander);
                    // Send the information about the command pressed to the loggin method.
                    logInformation(rollLeftCommand);
                break; 
            // case for D, roll the lander right by 0.5.
            case 100:
                    sendto(s, rollRightCommand, strlen(rollRightCommand), 0, (struct sockaddr *) &socketInLander, stringLengthLander);
                    // Send the information about the command pressed to the loggin method.
                    logInformation(rollRightCommand);
                break; 


            // 0, Turn of the engines.
            case 48:
                    sendto(s, increaseEngineToZeroCommand, strlen(increaseEngineToZeroCommand), 0, (struct sockaddr *) &socketInLander, stringLengthLander);
                    // Send the information about the command pressed to the loggin method.
                    logInformation(increaseEngineToZeroCommand);
                break; 
            // 1, Send the command to set the engines to 10 percent.
            case 49:
                    sendto(s, increaseEngineToTenCommand, strlen(increaseEngineToTenCommand), 0, (struct sockaddr *) &socketInLander, stringLengthLander);
                    // Send the information about the command pressed to the loggin method.
                    logInformation(increaseEngineToTenCommand);
                break; 
            // 2, Send the command to set the engines to 20 percent.
            case 50:
                    sendto(s, increaseEngineToTwentyCommand, strlen(increaseEngineToTwentyCommand), 0, (struct sockaddr *) &socketInLander, stringLengthLander);
                    // Send the information about the command pressed to the loggin method.
                    logInformation(increaseEngineToTwentyCommand);
                break; 
            // 3, Send the command to set the engines to 30 percent.
            case 51:
                    sendto(s, increaseEngineToThirtyCommand, strlen(increaseEngineToThirtyCommand), 0, (struct sockaddr *) &socketInLander, stringLengthLander);
                    // Send the information about the command pressed to the loggin method.
                    logInformation(increaseEngineToThirtyCommand);
                break; 
            // 4, Send the command to set the engines to 40 percent.
            case 52:
                    sendto(s, increaseEngineToFortyCommand, strlen(increaseEngineToFortyCommand), 0, (struct sockaddr *) &socketInLander, stringLengthLander);
                    // Send the information about the command pressed to the loggin method.
                    logInformation(increaseEngineToFortyCommand);
                break; 
            // 5, Send the command to set the engines to 50 percent.
            case 53:
                    sendto(s, increaseEngineToFiftyCommand, strlen(increaseEngineToFiftyCommand), 0, (struct sockaddr *) &socketInLander, stringLengthLander);
                    // Send the information about the command pressed to the loggin method.
                    logInformation(increaseEngineToFiftyCommand);
                break; 
            // 6, Send the command to set the engines to 60 percent.
            case 54:
                    sendto(s, increaseEngineToSixtyCommand, strlen(increaseEngineToSixtyCommand), 0, (struct sockaddr *) &socketInLander, stringLengthLander);
                    // Send the information about the command pressed to the loggin method.
                    logInformation(increaseEngineToSixtyCommand);
                break; 
            // 7, Send the command to set the engines to 70 percent.
            case 55:
                    sendto(s, increaseEngineToSeventyCommand, strlen(increaseEngineToSeventyCommand), 0, (struct sockaddr *) &socketInLander, stringLengthLander);
                    // Send the information about the command pressed to the loggin method.
                    logInformation(increaseEngineToSeventyCommand);
                break; 
            // 8, Send the command to set the engines to 80 percent.
            case 56:
                    sendto(s, increaseEngineToEightyCommand, strlen(increaseEngineToEightyCommand), 0, (struct sockaddr *) &socketInLander, stringLengthLander);
                    // Send the information about the command pressed to the loggin method.
                    logInformation(increaseEngineToEightyCommand);
                break;
            // 9, Send the command to set the engines to 90 percent.
            case 57:
                    sendto(s, increaseEngineToNinetyCommand, strlen(increaseEngineToNinetyCommand), 0, (struct sockaddr *) &socketInLander, stringLengthLander);
                    // Send the information about the command pressed to the loggin method.
                    logInformation(increaseEngineToNinetyCommand);
       
