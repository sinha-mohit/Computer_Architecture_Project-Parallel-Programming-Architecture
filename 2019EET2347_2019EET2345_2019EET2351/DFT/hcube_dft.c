/* Compile - mpicc -o hcube_dft hcube_dft.c -lm
 * Execute - mpirun -np 16  --tag-output hcube_dft */
/*http://computerk001.blogspot.com/2014/01/dspwrite-c-program-to-find-dft-of-given.html*/
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#define PI 3.14159
#define BCAST "Broadcast"
#define REDUC "Reduction"
#define SEND "sending"
#define RECV "Receiving"
void print_g_f(float *a, int size,int dir,int phase,int phase_no){

        int i = 0;
        //for (i = 0; i <(sizeof(a)/sizeof(int)); i++){
        printf("[%s : %s : phase - %d]: \n",phase ? BCAST:REDUC,dir ? SEND:RECV,phase_no);
        for (i = 0; i < size; i++){
                printf("  %f", a[i]);
        }
        printf("\n");
        printf("%d\n",i);
}


void print_g(int *a, int size,int dir,int phase, int phase_no){

        int i = 0;
        //for (i = 0; i <(sizeof(a)/sizeof(int)); i++){
        printf("\n");
        printf("[%s : %s : phase - %d]: \n",phase ? BCAST:REDUC,dir ? SEND:RECV,phase_no);
        for (i = 0; i < size; i++){
                printf("  %d", a[i]);
        }
        printf("\n");
        printf("%d\n",i);
}

void creat_ip_arr(int *ar, int size){

        int n = 0;

        for (int c = 0; c < size; c++)
        {
                n = rand() % 1000 + 1;
                ar[c] = n;
        }

        printf("Input array is ");
        for (int c = 0; c < size; c++)
        {
                printf("%d ", ar[c]);
        }
}

void display(int *a, int size, int init,int phase){

        int i = 0;
	//printf("Phase %d\n",phase);
        //for (i = 0; i <(sizeof(a)/sizeof(int)); i++){
        for (i = 0; i < size; i++){
                printf("%d  ", a[i]);
        }
        printf("\n");
        printf("size %d\n",i);
	printf("Init %d\n",init);
}

void copy_arr(int *master, int *slave, int length, int start_m, int start_s){

        int i = 0;
        int j = start_s;

        //slave = (int *)malloc(sizeof(int)*length);
        for (i=start_m; i < (start_m+length);i++){
                slave[j] = master[i];
                j++;
        }
}


void append_arr(int *arr,int pos,int value){

	arr[pos] = value;
}

void extract_arr(int *arr, int pos, int *var){
	
	*var = arr[pos];
}



int main(int argc, char **argv){

	int output = 0;
	int *arr = NULL;
        //int arr[] = {247, 198, 235, 85, 65, 216, 92, 35, 53, 249};
        //int arr[] = {10,5,6,7,11};
        //int arr[] = {10,5,6,7,11,8,12,6};
        //int arr[] = {10,5,6,7,11,8,12,6,13,17,15,19,20,25,23,21,54,78,41,97,14,64,52,63,46,35,68,45,88,65,98,29};
        //int arr[] = {10,5,6,7,11,8,12,6,13,17,15,19,20,25,23,21};
        int rank;
        int world_size;
	int mask = 1;
	int phase = 0;
	int phase_max = 0;
	int sender = 0;
	int recepient = 0;
	int arr_size = 8;
	int pres_size = 0;
	int pres_init = 0;
	int pres_size_tmp = 0;
	int pres_init_tmp = 0;
	int trail_size = 0;
	int recv_size = 0;
	int *pres = NULL;
	int *pres_tmp = NULL;
	int *pres_recv_tmp = NULL;
	int *send_arr = NULL;
	int *sor_arr = NULL;
	MPI_Status stat;
	float X_Real[arr_size], X_Imag[arr_size];
	float send_sum[2];
	float recv_sum[2];
	int n,k;
	float arg = 0.0;
	double start,end;
	clock_t time;

        MPI_Init(NULL, NULL);

        MPI_Comm_size(MPI_COMM_WORLD, &world_size);

        MPI_Comm_rank(MPI_COMM_WORLD, &rank);

        printf("Hello world from processor rank %d out of %d processors\n", rank, world_size);
        if (rank == 0) {
                arr = (int*)malloc(sizeof(int)*arr_size);
                creat_ip_arr(arr,arr_size);
        }

	time = clock();

	for(k=0;k<arr_size;k++){
  		
		X_Real[k] = X_Imag[k] = 0.0;
// book algorithm starts from here...distribution phase
		mask = 1;
		for(phase = 1; phase <= (log(world_size)/log(2)); phase++){ //world size = 16 4phasse

			phase_max = pow(2,phase);

			if (rank >= 0 && rank <= ((phase_max/2)-1)){

				recepient = rank | mask;
				printf("[Broadcast : sending] : Phase is %d Sender is %d and Recepient is %d\n", phase,rank, recepient);
				if (phase == 1){
					pres_size = arr_size/2;
					pres_init = 0;
					pres = (int*)malloc(sizeof(int)*pres_size);
					copy_arr(arr,pres,pres_size,0,0);
					trail_size = arr_size - pres_size;
					send_arr = (int*)malloc(sizeof(int)*(trail_size+1));
					copy_arr(arr,send_arr,trail_size,pres_size,0);
					append_arr(send_arr,trail_size,pres_size);
				} else {
					pres_size_tmp = pres_size/2;
					//pres_init_tmp = pres_size_tmp;
					pres_tmp = (int*)malloc(sizeof(int)*pres_size_tmp);
					copy_arr(pres,pres_tmp,pres_size_tmp,0,0);
					trail_size = pres_size - pres_size_tmp;
					send_arr = (int*)malloc(sizeof(int)*(trail_size+1));
					copy_arr(pres,send_arr,trail_size,pres_size_tmp,0);
					append_arr(send_arr,trail_size,pres_size_tmp+pres_init);
					free(pres);
					pres_size = pres_size_tmp;
					//pres_init = pres_init_tmp;
					pres = (int*)malloc(sizeof(int)*pres_size);
					copy_arr(pres_tmp,pres,pres_size,0,0);
					free(pres_tmp);
				}
				//display(send_arr,trail_size+1,pres_init,phase);
	                        print_g(send_arr,trail_size,1,1,phase);
				MPI_Send(send_arr,trail_size+1,MPI_INT,recepient,1,MPI_COMM_WORLD);
			}

			if (rank >= phase_max/2 && rank <= phase_max-1){

				MPI_Status status;
				sender = rank & (~mask);
				printf("[Broadcast : Receiving] : Phase is %d Sender is %d and Recepient is %d\n", phase,sender,rank);

				// Probe for an incoming message from process zero
				MPI_Probe(sender, 1, MPI_COMM_WORLD, &status); // waiting

				// When probe returns, the status object has the size and other
				// attributes of the incoming message. Get the message size
				MPI_Get_count(&status, MPI_INT, &pres_size); // asking/getting size

				// Allocate a buffer to hold the incoming numbers
				pres_recv_tmp = (int*)malloc(sizeof(int) * pres_size);
				pres = (int*)malloc(sizeof(int) * (pres_size-1));

				MPI_Recv(pres_recv_tmp,pres_size,MPI_INT,sender,1,MPI_COMM_WORLD,&stat);
	                        print_g(pres_recv_tmp,pres_size,0,1,phase);

				copy_arr(pres_recv_tmp,pres,pres_size-1,0,0);
				extract_arr(pres_recv_tmp,pres_size-1,&pres_init);
				pres_size = pres_size - 1;
				free(pres_recv_tmp);
				//display(pres,pres_size,pres_init,phase);
			}

			mask = mask << 1;
		}


		display(pres,pres_size,pres_init,phase);
		
		for(n=0; n<pres_size; n++) {
			
			arg = 2*PI*k*(n+pres_init)/arr_size;
			X_Real[k] = X_Real[k] + (pres[n]*cos(arg));
			X_Imag[k] = X_Imag[k] + (pres[n]*sin(arg));
		}

 
	
		mask = world_size/2;
// now reverse algorithm for accumulation
		for (phase = (log(world_size)/log(2)); phase >= 1; phase--){

			phase_max = pow(2,phase);

			if (rank >= phase_max/2 && rank <= phase_max-1){

				recepient = rank & (~mask);
				printf("[Reduction : sending] : Phase is %d Sender is %d and Recepient is %d\n", phase,rank, recepient);
				bzero(send_sum,sizeof(send_sum));
				send_sum[0] = X_Real[k];
				send_sum[1] = X_Imag[k];
				print_g_f(send_sum,2,1,0,phase);
				MPI_Send(send_sum,2,MPI_INT,recepient,1,MPI_COMM_WORLD);
			}

			if(rank >= 0 && rank <= ((phase_max/2)-1)){
				MPI_Status status;
				sender = rank | mask;
				printf("[Reduction : Receiving] : k is %d Phase is %d Sender is %d and Recepient is %d\n",k, phase,sender,rank);

				//MPI_Probe(sender, 1, MPI_COMM_WORLD, &status);

				//MPI_Get_count(&status, MPI_INT, &recv_size);

				//recv_arr = (int*)malloc(sizeof(int)*recv_size);
				bzero(recv_sum,sizeof(recv_sum));
				MPI_Recv(recv_sum,2,MPI_INT,sender,1,MPI_COMM_WORLD,&stat);
				print_g_f(recv_sum,2,0,0,phase);
				printf("[Reduction : Receiving] : 2nd k is %d Phase is %d Sender is %d and Recepient is %d\n",k, phase,sender,rank);
				X_Real[k] = X_Real[k] + recv_sum[0];
				X_Imag[k] = X_Imag[k] + recv_sum[1];

			}
			mask = mask >> 1;
		}
		X_Imag[k] = X_Imag[k]*(-1.0);
	}

	time = clock() - time;

	if(rank == 0){
		//display(pres,pres_size);
		printf("\nThe %d point DFT of given sequence is:\n",arr_size);
		printf("\n\n\tReal X(k)\t\tImaginary X(k)\n");
   		for(k=0; k<arr_size; k++){
   			printf("\nX(%d)= %f\t\t\t%f\t\t",k,X_Real[k],X_Imag[k]);
		}
		printf("\nTime taken is %f\n",((double)time)/CLOCKS_PER_SEC);

	}

	MPI_Finalize();
}
