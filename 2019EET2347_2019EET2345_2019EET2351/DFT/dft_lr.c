/* Compile - mpicc -o dft_lr dft_lr.c -lm
 * Execute - mpirun -np 8  --tag-output dft_lr */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <time.h>
#define PI 3.14159
#define BCAST "Broadcast"
#define REDUC "Reduction"
#define SEND "sending"
#define RECV "Receiving"

void display(int *a, int r, int size);

void print_g_f(float *a, int size,int dir,int phase){

        int i = 0;
        //for (i = 0; i <(sizeof(a)/sizeof(int)); i++){
        printf("[%s : %s]: \n",phase ? BCAST:REDUC,dir ? SEND:RECV);
        for (i = 0; i < size; i++){
                printf("  %f", a[i]);
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

void print_g(int *a, int size,int dir,int phase){

        int i = 0;
        //for (i = 0; i <(sizeof(a)/sizeof(int)); i++){
        printf("[%s : %s]: \n",phase ? BCAST:REDUC,dir ? SEND:RECV);
        for (i = 0; i < size; i++){
                printf("  %d", a[i]);
        }
        printf("\n");
        printf("%d\n",i);
}

                                                                                                                                              
void display(int *a, int r, int size){

        int i = 0;
        //for (i = 0; i <(sizeof(a)/sizeof(int)); i++){
        printf("rank %d \n",r);
        for (i = 0; i < size; i++){
                printf("%d  ", a[i]);
        }
        printf("\n");
        printf("%d\n",i);
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


int main(int argc, char** argv) {

	int output = 0;
        //int arr[] = {10,5,6,7,11,8,12,6};
        //int arr[] = {10,5,6,7,11,8,12};
        //int arr[] = {10,5,6,7,11,8,12,3,13,17,15,19,20,25,23,21,4};
        //int arr[] = {10,5,6,7,11,8,12,3,13,17,15,19,20,25,23};
	int *arr = NULL;
	int rank;
	int world_size;
	int size;
	int arr_size = 4;
	int extra = 0;
	int *sub_array = NULL;
        clock_t time;
        float arg = 0.0;
        float send_sum[2];
        float recv_sum[2];
        int k;

	MPI_Init(NULL, NULL);

	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (world_size%2 != 0 || world_size == 2){
		printf("Invalid number of processors.\n");
		exit(1);
	}

	//display(arr,rank);
	//arr_size = (sizeof(arr)/sizeof(arr[0]));
	
	if (rank == 0) {
		arr = (int*)malloc(sizeof(int)*arr_size);
		creat_ip_arr(arr,arr_size);
	}

	size = arr_size/world_size;
	extra = arr_size%world_size;

	float X_Real[arr_size], X_Imag[arr_size];

	printf("arr size %d, size %d, extra %d\n",arr_size, size, extra);	
	//sub_array = malloc(size * sizeof(int));
	//MPI_Scatter(arr, size, MPI_INT, sub_array, size, MPI_INT, 0, MPI_COMM_WORLD);

        time = clock();

        for(k=0;k<arr_size;k++){

                X_Real[k] = X_Imag[k] = 0.0;

		if (rank == 0){
			int *rec_1 = NULL;
			int *rec_2 = NULL;
			int pos = 0;
			int sarr_size = 0;
			int mid = 0;
			int total_1 = 0;
			int total_2 = 0;
			int total = 0;
			MPI_Status stat;
			int *sor_arr = NULL;
			int *sub_arr = NULL;
			int *merge_1 = NULL;
			int *merge_2 = NULL;

			sor_arr = (int *)malloc(sizeof(int)*size);
			copy_arr(arr,sor_arr,size,0,0);
			sarr_size = (((world_size/2) - rank) * size) + extra;
			sub_arr = (int *)malloc(sizeof(int) * (sarr_size+1));
			copy_arr(arr,sub_arr,sarr_size,size,0);
			append_arr(sub_arr,sarr_size,size);
			//display(sub_arr,rank,(sarr_size+1));
			print_g(sub_arr,sarr_size+1,1,1);
			MPI_Send(sub_arr,sarr_size+1,MPI_INT,1,1,MPI_COMM_WORLD);
			free(sub_arr);
			sarr_size = ((world_size - ((world_size/2)+1)) * size);
			sub_arr = (int *)malloc(sizeof(int) * (sarr_size+1));
			copy_arr(arr,sub_arr,sarr_size,arr_size-sarr_size,0);
			append_arr(sub_arr,sarr_size,arr_size-sarr_size);
			//display(sub_arr,rank,(sarr_size+1));
			print_g(sub_arr,sarr_size+1,1,1);
			MPI_Send(sub_arr,sarr_size+1,MPI_INT,world_size-1,1,MPI_COMM_WORLD);
			free(sub_arr);

			printf("Rank is %d \n",rank);
			display(sor_arr,rank,size);
			printf("Starting position is %d\n",pos);

			for(int n=0; n<size; n++) {

				arg = 2*PI*k*(n+pos)/arr_size;
				X_Real[k] = X_Real[k] + (sor_arr[n]*cos(arg));
				X_Imag[k] = X_Imag[k] + (sor_arr[n]*sin(arg));
			}
			
			MPI_Recv(recv_sum,2,MPI_INT,1,1,MPI_COMM_WORLD,&stat);
			print_g_f(recv_sum,2,0,0);
			X_Real[k] = X_Real[k] + recv_sum[0];
			X_Imag[k] = X_Imag[k] + recv_sum[1];

			MPI_Recv(recv_sum,2,MPI_INT,world_size-1,1,MPI_COMM_WORLD,&stat);
			print_g_f(recv_sum,2,0,0);
			X_Real[k] = X_Real[k] + recv_sum[0];
			X_Imag[k] = X_Imag[k] + recv_sum[1];


		} else if (rank == world_size/2){
			int *sor_arr = NULL;
			int *rec_arr = NULL;
			int rec_size = 0;
			int pos = 0;
			MPI_Status stat;
			MPI_Status status;

			MPI_Probe((world_size/2)-1, 1, MPI_COMM_WORLD, &status);

			MPI_Get_count(&status, MPI_INT, &rec_size);

			rec_arr = (int *)malloc(sizeof(int)*rec_size);
			sor_arr = (int *)malloc(sizeof(int)*(rec_size-1));
			MPI_Recv(rec_arr,rec_size,MPI_INT,(world_size/2)-1,1,MPI_COMM_WORLD,&stat);
			print_g(rec_arr,rec_size,0,1);

			pos = rec_arr[rec_size-1];
			copy_arr(rec_arr,sor_arr,rec_size-1,0,0);
			free(rec_arr);
			printf("Rank is %d\n",rank);
			display(sor_arr,rank,rec_size-1);
			printf("Starting position is %d\n",pos);

                        for(int n=0; n<rec_size-1; n++) {

                                arg = 2*PI*k*(n+pos)/arr_size;
                                X_Real[k] = X_Real[k] + (sor_arr[n]*cos(arg));
                                X_Imag[k] = X_Imag[k] + (sor_arr[n]*sin(arg));
                        }
                        
                        send_sum[0] = X_Real[k];
                        send_sum[1] = X_Imag[k];

			print_g_f(send_sum,2,1,0);
	                MPI_Send(send_sum,2,MPI_INT,(world_size/2)-1,1,MPI_COMM_WORLD);

		} else if (rank == ((world_size/2)+1)){
			int *sor_arr = NULL;
			int *rec_arr = NULL;
			int rec_size = 0;
			int pos = 0;
			MPI_Status stat;
			MPI_Status status;

			MPI_Probe(((world_size/2)+2) % world_size, 1, MPI_COMM_WORLD, &status);

			MPI_Get_count(&status, MPI_INT, &rec_size);

			rec_arr = (int *)malloc(sizeof(int)*rec_size);
			sor_arr = (int *)malloc(sizeof(int)*(rec_size-1));

			MPI_Recv(rec_arr,rec_size,MPI_INT,((world_size/2)+2) % world_size ,1,MPI_COMM_WORLD, &stat);
			print_g(rec_arr,rec_size,0,1);

			//display(rec_arr,rank,rec_size);
			pos = rec_arr[rec_size-1];
			copy_arr(rec_arr,sor_arr,rec_size-1,0,0);
			free(rec_arr);
			printf("Rank is %d\n",rank);
			display(sor_arr,rank,size);
			printf("Starting position is %d\n",pos);

			for(int n=0; n<size; n++) {

				arg = 2*PI*k*(n+pos)/arr_size;
				X_Real[k] = X_Real[k] + (sor_arr[n]*cos(arg));
				X_Imag[k] = X_Imag[k] + (sor_arr[n]*sin(arg));
			}

			send_sum[0] = X_Real[k];
			send_sum[1] = X_Imag[k];

			print_g_f(send_sum,2,1,0);
			MPI_Send(send_sum,2,MPI_INT,(((world_size/2)+2) % world_size) ,1,MPI_COMM_WORLD);


		} else if (rank > 0 && rank < world_size/2){
			int *rec_arr = NULL;
			int *rec_arr_tmp = NULL;
			int send_3 = 0;
			int rec_size = 0;
			int sarr_size = 0;
			int pos = 0;
			int mid_1 = 0;
			int total_1 = 0;
			MPI_Status stat;
			int *sor_arr = NULL;
			int *sub_arr = NULL;
			int *merge_1 = NULL;
			MPI_Status status;

			MPI_Probe(rank-1, 1, MPI_COMM_WORLD, &status);

			MPI_Get_count(&status, MPI_INT, &rec_size);

			rec_arr = (int *)malloc(sizeof(int)*rec_size);
			rec_arr_tmp = (int *)malloc(sizeof(int)*(rec_size-1));


			MPI_Recv(rec_arr,rec_size,MPI_INT,rank-1,1,MPI_COMM_WORLD,&stat);
			print_g(rec_arr,rec_size,0,1);
			pos = rec_arr[rec_size-1];
			copy_arr(rec_arr,rec_arr_tmp,rec_size-1,0,0);
			free(rec_arr);

			sor_arr = (int *)malloc(sizeof(int)*size);
			copy_arr(rec_arr_tmp,sor_arr,size,0,0);
			//if (rank == 2) goto end;
			sarr_size = (((world_size/2) - rank) * size) + extra;
			sub_arr = (int *)malloc(sizeof(int) * (sarr_size+1));
			copy_arr(rec_arr_tmp,sub_arr,sarr_size,size,0);
			append_arr(sub_arr,sarr_size,size+pos);

			print_g(sub_arr,sarr_size+1,1,1);
			MPI_Send(sub_arr,sarr_size+1,MPI_INT,rank+1,1,MPI_COMM_WORLD);
			free(rec_arr_tmp);
			free(sub_arr);

			printf("Rank is %d\n",rank);
			display(sor_arr,rank,size);
			printf("Starting position is %d\n",pos);

                        for(int n=0; n<size; n++) {

                                arg = 2*PI*k*(n+pos)/arr_size;
                                X_Real[k] = X_Real[k] + (sor_arr[n]*cos(arg));
                                X_Imag[k] = X_Imag[k] + (sor_arr[n]*sin(arg));
                        }

			MPI_Recv(recv_sum,2,MPI_INT,rank+1,1,MPI_COMM_WORLD,&stat);
			print_g_f(recv_sum,2,0,0);
                        X_Real[k] = X_Real[k] + recv_sum[0];
                        X_Imag[k] = X_Imag[k] + recv_sum[1];

                        send_sum[0] = X_Real[k];
                        send_sum[1] = X_Imag[k];

			print_g_f(send_sum,2,1,0);
			MPI_Send(send_sum,2,MPI_INT,rank-1,1,MPI_COMM_WORLD);


		} else if (rank > ((world_size/2)+1) && rank <= (world_size - 1)){
			int *rec_arr = NULL;
			int *rec_arr_tmp = NULL;
			int send_4 = 0;
			int rec_size = 0;
			int sarr_size = 0;
			int pos = 0;
			int mid_2 = 0;
			int total_1 = 0;
			MPI_Status stat;
			int *sor_arr = NULL;
			int *sub_arr = NULL;
			int *merge_1 = NULL;
			MPI_Status status;

			MPI_Probe((rank+1)%world_size, 1, MPI_COMM_WORLD, &status);

			MPI_Get_count(&status, MPI_INT, &rec_size);

			rec_arr = (int *)malloc(sizeof(int)*rec_size);
			rec_arr_tmp = (int *)malloc(sizeof(int)*(rec_size-1));

			MPI_Recv(rec_arr,rec_size,MPI_INT,(rank+1)%world_size,1,MPI_COMM_WORLD,&stat);
			print_g(rec_arr,rec_size,0,1);
			//display(rec_arr,rank,rec_size);
			pos = rec_arr[rec_size-1];
			copy_arr(rec_arr,rec_arr_tmp,rec_size-1,0,0);
			free(rec_arr);

			sor_arr = (int *)malloc(sizeof(int)*size);
			copy_arr(rec_arr_tmp,sor_arr,size,0,0);
			sarr_size = ((rank - ((world_size/2)+1)) * size);
			sub_arr = (int *)malloc(sizeof(int) * (sarr_size+1));
			copy_arr(rec_arr_tmp,sub_arr,sarr_size,size,0);
			append_arr(sub_arr,sarr_size,size+pos);


			//display(sub_arr,rank,sarr_size+1);
			print_g(sub_arr,sarr_size+1,1,1);
			MPI_Send(sub_arr,sarr_size+1,MPI_INT,rank-1,1,MPI_COMM_WORLD);
			free(rec_arr_tmp);
			free(sub_arr);

			printf("Rank is %d\n",rank);
			display(sor_arr,rank,size);
			printf("Starting position is %d\n",pos);
                        for(int n=0; n<size; n++) {

                                arg = 2*PI*k*(n+pos)/arr_size;
                                X_Real[k] = X_Real[k] + (sor_arr[n]*cos(arg));
                                X_Imag[k] = X_Imag[k] + (sor_arr[n]*sin(arg));
                        }

                        MPI_Recv(recv_sum,2,MPI_INT,rank-1,1,MPI_COMM_WORLD,&stat);
			print_g_f(recv_sum,2,0,0);
                        X_Real[k] = X_Real[k] + recv_sum[0];
                        X_Imag[k] = X_Imag[k] + recv_sum[1];

                        send_sum[0] = X_Real[k];
                        send_sum[1] = X_Imag[k];

			print_g_f(send_sum,2,1,0);
                        MPI_Send(send_sum,2,MPI_INT,(rank+1)%world_size,1,MPI_COMM_WORLD);

		}
		X_Imag[k] = X_Imag[k]*(-1.0);
	}

        time = clock() - time;

	//sleep(600);
	// Finalize the MPI environment.
	if(rank == 0){
                //display(pres,pres_size);
                printf("\nThe %d point DFT of given Input sequence is:\n",arr_size);
                printf("\n\n\tReal Part of X(k)\t\tImaginary Part of X(k)\n");
                for(k=0; k<arr_size; k++){
                        printf("\nX(%d)= %f\t\t\t%f\t\t",k,X_Real[k],X_Imag[k]);
                }
                printf("\nTime taken is %f\n",((double)time)/CLOCKS_PER_SEC);

        }

	MPI_Finalize();
}
