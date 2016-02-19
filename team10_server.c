#include<stdio.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<sys/select.h>
#include<time.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<string.h>



#define DEFAULT_PORT 4548
#define TIMEOUT 1
#define WINDOW_SIZE 1

#define FILE_NAME_MAX 64
#define MAX_FILE_LEN 512



int main(int argc, char *argv[])
{


   //List of important character arrays used
   int i,j,k;
   int num, num_of_bytes, port, client_len, sock_id;
   int data_size = 516, begin = 1, char_count;
   char opcode; //Opcode in the packet.


   struct sockaddr_in server, client; //define the client and server sockets.
   char client_data [516]; // Useful Data received from the client.
   char file_data[ MAX_FILE_LEN ]; //File contents
   char file_name[ FILE_NAME_MAX ]; //File name.
   char data[516]; //data to be sent to client
   char transmit_data[ WINDOW_SIZE ][512];  // has transmitted data for each packet in the window.
   
   int packet_number [ WINDOW_SIZE ]; // Stores the block numbers of packets in the window.
   int packet_time_sent [ WINDOW_SIZE ]; // Stores the time when these packets were sent.
   char block_no;//sent by server
   char *temp;
   static block_number, last_ack_packet, completed,last_packet;
   static current, prev;//sliding window marker
   static last_window,recent_window; //window variables. Maintain the last ACKed packet and the most recent packet sent in the window.

   char c;
                 
     
   FILE *fp;  // File Pointer

   fd_set readfd; //readset buffer.
   int ans_select; //result of select.

   time_t tm;
   struct timeval tv;
   tv.tv_sec = 1;
   tv.tv_usec = 0; //total time of 1 second

  int error;
 
        if (argc ==2 ) //if user has entered port
                port = atoi(argv[1]);
        else if (argc == 1) //if user has not entered port specify default port
                port = DEFAULT_PORT;
        else //Any other format
                {fprintf(stderr,"format should be ./a.out <port_number>\n");
                 exit(1);
                }

        bzero((char*) &server,sizeof(struct sockaddr_in));
        server.sin_family = AF_INET;//set family
        server.sin_port = htons(port); //set the port
        server.sin_addr.s_addr = htonl(INADDR_ANY); //set the ip address.


        if((sock_id= socket(AF_INET, SOCK_DGRAM, 0))== -1) //Socket Function
        {
        fprintf(stderr, "Socket cannot be created \n");
        exit(1);
        }


        if(bind(sock_id, (struct sockaddr *)&server, sizeof(server))== -1) //bind function 
			       {
        fprintf(stderr, "Cannot bind to the socket \n");
         exit(1);
          }

         printf("Server can accept Clients \n"); // Server is ready


while(1)
{
int timedout = 0 ;
client_len = sizeof(client);

  for (k=0; k<WINDOW_SIZE; k++)
   {
        (void)time(&tm);//return value is also stored in the memory pointed tm
        if ( ((int) tm - packet_time_sent[k])> TIMEOUT && begin == 0) //timeout condition and its not the beginning

        {
                timedout = 1 ;
                printf("TIMEOUT\n");
            data[0] = 0; 
			data[1] = 3;
            data[2] = 0; 
			data[3] = packet_number [k];
            packet_time_sent[k] = (int) tm;

            if(sendto(sock_id, data, data_size, 0, (struct sockaddr *)&client, client_len) == -1) //send to client.
            {
                fprintf(stderr, "Error in sending.\n");
                exit(1);
            }
            printf(" DATA Packet Block_no = %d is sent\n", data[3]);
            break;


        }
     }

   if (timedout ==1) //if time out has occured
        continue;


FD_ZERO (&readfd);

FD_SET (sock_id, &readfd);

ans_select = select (sock_id+1,&readfd,0,0,&tv);

  if(ans_select <= 0)//go back to start of loop
        {
        continue;
        }

  if ((num = recvfrom (sock_id,client_data,516,0,(struct sockaddr *) &client,(socklen_t *) &client_len))< 0)
     {
        fprintf(stderr, "Cannot receive connection \n");
        exit(1);
     }


if (completed == 1 && *(client_data+1) == 4 ) //after all packets have been sent only ack remain
        {
            block_no = *(client_data +3);
            printf(" ACK received for block # %d\n", block_no);
            continue; //back to start of loop
        }

     temp = client_data;
     temp++; //getting opcode from the packet.
     opcode = *temp;
     temp++;

switch (opcode)

{ case 1:
        strcpy(file_name,&client_data[2]);
        printf("Read request received for file %s \n",file_name);

        /*reading content from file */
        fp = fopen(file_name, "r");
        if (fp == NULL) //File not found
        {
                error = 1;
                data[0] = 0;
				data[1] = 5; //make erros packet
                data[2] = 0;
				data[3] = 1;//error=file not found
                                                                                      
				         printf("file not found");
                strcpy(&data[4],"File not Found");
                data_size = sizeof(data);
                if (sendto (sock_id,data,data_size,0, (struct sockaddr *)&client, client_len) == -1)
              {
                 fprintf(stderr, "error in sending\n");
                 exit(1);
              }

              break;
         }

        else
        {
                completed =0;
                error =0;
                begin=0;
                block_number =1;//start block number from 1
                printf("file opened successfully\n");

                        for(i=0;i<WINDOW_SIZE; i++)
                        {
                                num_of_bytes = fread(file_data, 1, 512, fp);


                                if (num_of_bytes < 512)
                                {    completed = 1; begin = 1; //Set to completed and begin again.
                                }
                                /*make data package*/
                                data[0]= 0;
								data[1]= 3;
                                data[2]= 0;
								data[3]= block_number;

								current = 0; prev = 3; //save the current and previous windows.
                                strcpy(&data[4], file_data);
                                strcpy(transmit_data[i], &data[4]);
                                (void) time(&tm);
                                packet_time_sent [i] = (int) tm;
								packet_number [i] =block_number;
								printf("DATA packet Block_num = %d packet is sent of size %d\n",block_number,num_of_bytes);//print the packet to be sent.
								data_size = num_of_bytes + 4; //file size + four additional bytes.
                                 if(sendto(sock_id, data, data_size, 0, (struct sockaddr *)&client, client_len) == -1) //send to client.
                                {
                                     fprintf(stderr, "Error in sending data.\n");
                                     exit(1);
                                 }
                                 if (completed == 1)
                                 {
                                     recent_window = block_number;
                                     break;
                                 }
                        block_number++;

                        }
                recent_window =block_number;


        }
        break;

case 4:  	
		if (completed ==1)
        {
                begin = 1; //set begin to 1 A FILE TRANSFER COMPLETE
                printf("Transfer Complete");
				break;
        }
        temp++;////increment pointer to next
        block_no = *temp;////block number recieved
        last_window =block_no; //set to last recieved window;
		
        if(last_ack_packet != block_no) //if ACK recieved is not duplicated
           { if (last_window == 1)
                        recent_window = last_window + WINDOW_SIZE;
                 else
                        recent_window = recent_window +1;
			    
                printf (" ACK recieved for block # %d\n", block_no);
                if(error == 1)
                { error =0;
                 break;
                }
                num_of_bytes = fread(file_data,1,512,fp);
               

                data[0] = 0;
				data[1] = 3;
                data[2] = 0;
				data[3] = recent_window;

                strcpy(&data[4], file_data);
                if(current != WINDOW_SIZE - 1)
                {
                      prev = current;
                      current++;
                }
                else
                {
                     prev = current;
                     current = 0;
                }
                strcpy(transmit_data[prev], &data[4]);
                packet_number [prev] = block_no; //save the block number.
                (void) time (&tm);
                packet_time_sent [prev] = (int) tm; //save the time.
	
				printf("DATA packet Block_num = %d packet is sent of size %d\n",recent_window,num_of_bytes);
                data_size = num_of_bytes + 4;
                if(sendto(sock_id, data, data_size, 0, (struct sockaddr *) &client, client_len) == -1)
                {
                    fprintf(stderr, "Cannot send datagram 3\n");
                     exit(1);
                }
				 if (num_of_bytes < 512)
				 {  completed = 1; begin = 1; //Completed and begin again.
                    last_packet =  recent_window ;
				 }

                last_ack_packet = block_no;
				break;
        }
		
        else
        {
                printf("Duplicate ACK for block no %d\n",block_no);//Duplicate ACK received.
              break;
        }
       
		break;

default :
        printf("not valid operation \n");
        break;

   }

}
fclose(fp);
close(sock_id);


return 0;

}



                                                                                                                     