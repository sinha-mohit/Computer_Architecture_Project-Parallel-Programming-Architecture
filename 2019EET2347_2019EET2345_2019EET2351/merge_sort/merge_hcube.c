/* Compile - mpicc -o merge_hcube merge_hcube.c -lm
 * Execute - mpirun -np 16  --tag-output merge_hcube */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#define BCAST "Broadcast"
#define REDUC "Reduction"
#define SEND "sending"
#define RECV "Receiving"

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


void display(int *a, int size){

        int i = 0;
        //for (i = 0; i <(sizeof(a)/sizeof(int)); i++){
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


void merge(int *old, int *new, int p, int q, int r){

        int i = p;
        int op = p;
        int j = q;
        int k = q+1;
        int m = r;
        int n = 0;

        //printf("\n");
        //printf("p %d  q %d r %d\n",p,q,r);

        //printf("\n");
        //display(old);
        while(i<=j && k<=m){

                //printf("i %d k %d\n",i,k);
                if (old[i] <= old[k]){
                        new[p] = old[i];
                        i++;
                } else {
                        new[p] = old[k];
                        k++;
                }
                p++;
        }

	while (i <= j){
                new[p] = old[i];
                p++;
                i++;
        }

        while (k <= m){
                new[p] = old[k];
                p++;
                k++;
        }

        for (i=op; i<=r; i++){
        //for (i=0; i<=r; i++){
                old[i] = new[i];
        }
}


void merge_sort(int *unsor, int *sor, int l, int h){

        int mid = 0;
        if (l < h){
                mid = (l+h)/2;
                merge_sort(unsor,sor,l,mid);
                merge_sort(unsor,sor,mid+1,h);
                merge(unsor,sor,l,mid,h);
                printf("\n");
                //display(sor);
        }
}



int main(int argc, char **argv){

	int output = 0;
	int *arr = NULL;
        //int arr[] = {247, 198, 235, 85, 65, 216, 92, 35, 53, 249};
        //int arr[] = {10,5,6,7,11};
        //int arr[] = {10,5,6,7,11,8,12,6};
        //int arr[] = {10,5,6,7,11,8,12,6,13,17,15,19,20,25,23,21,54,78,41,97,14,64,52,63,46,35,68,45,88,65,98,29};
        int rank;
        int world_size;
	int mask = 1;
	int phase = 0;
	int phase_max = 0;
	int sender = 0;
	int recepient = 0;
	int arr_size = 32;
	int pres_size = 0;
	int pres_size_tmp = 0;
	int trail_size = 0;
	int recv_size = 0;
	int *pres = NULL;
	int *pres_tmp = NULL;
	int *send_arr = NULL;
	int *recv_arr = NULL;
	int *sor_arr = NULL;
	MPI_Status stat;
	clock_t time;

        MPI_Init(NULL, NULL);

        MPI_Comm_size(MPI_COMM_WORLD, &world_size);

        MPI_Comm_rank(MPI_COMM_WORLD, &rank);

        //printf("Hello world from processor rank %d out of %d processors\n", rank, world_size);

        //arr_size = (sizeof(arr)/sizeof(arr[0]));
	if (rank == 0) {
		arr = (int*)malloc(sizeof(int)*arr_size);
                creat_ip_arr(arr,arr_size);
		time = clock();
        }


	for(phase = 1; phase <= (log(world_size)/log(2)); phase++){

		phase_max = pow(2,phase);

		if (rank >= 0 && rank <= ((phase_max/2)-1)){

			recepient = rank | mask;
			//printf("[Broadcast : sending] : Phase is %d Sender is %d and Recepient is %d\n", phase,rank, recepient);
			if (phase == 1){
				pres_size = arr_size/2;
				pres = (int*)malloc(sizeof(int)*pres_size);
				copy_arr(arr,pres,pres_size,0,0);
				trail_size = arr_size - pres_size;
				send_arr = (int*)malloc(sizeof(int)*trail_size);
				copy_arr(arr,send_arr,trail_size,pres_size,0);
			} else {
				pres_size_tmp = pres_size/2;
				pres_tmp = (int*)malloc(sizeof(int)*pres_size_tmp);
				copy_arr(pres,pres_tmp,pres_size_tmp,0,0);
				trail_size = pres_size - pres_size_tmp;
				send_arr = (int*)malloc(sizeof(int)*trail_size);
				copy_arr(pres,send_arr,trail_size,pres_size_tmp,0);
				free(pres);
				pres_size = pres_size_tmp;
				pres = (int*)malloc(sizeof(int)*pres_size);
				copy_arr(pres_tmp,pres,pres_size,0,0);
				free(pres_tmp);
			}
			print_g(send_arr,trail_size,1,1,phase);
	                MPI_Send(send_arr,trail_size,MPI_INT,recepient,1,MPI_COMM_WORLD);
		}

		if (rank >= phase_max/2 && rank <= phase_max-1){
			
			MPI_Status status;
			sender = rank & (~mask);
			printf("[Broadcast : Receiving] : Phase is %d Sender is %d and Recepient is %d\n", phase,sender,rank);

			// Probe for an incoming message from process zero
			MPI_Probe(sender, 1, MPI_COMM_WORLD, &status);

			// When probe returns, the status object has the size and other
			// attributes of the incoming message. Get the message size
			MPI_Get_count(&status, MPI_INT, &pres_size);

			// Allocate a buffer to hold the incoming numbers
			pres = (int*)malloc(sizeof(int) * pres_size);

			MPI_Recv(pres,pres_size,MPI_INT,sender,1,MPI_COMM_WORLD,&stat);
			print_g(pres,pres_size,0,1,phase);
		}

		mask = mask << 1;
	}


	sor_arr = (int *)malloc(sizeof(int)*pres_size);
	merge_sort(pres,sor_arr,0,pres_size-1);
	free(sor_arr);
	display(pres,pres_size);


	mask = world_size/2;

	for (phase = (log(world_size)/log(2)); phase >= 1; phase--){

		phase_max = pow(2,phase);

		if (rank >= phase_max/2 && rank <= phase_max-1){

			recepient = rank & (~mask);
                        printf("[Reduction : sending] : Phase is %d Sender is %d and Recepient is %d\n", phase,rank, recepient);

			print_g(pres,pres_size,1,0,phase);
                        MPI_Send(pres,pres_size,MPI_INT,recepient,1,MPI_COMM_WORLD);
		}
		if(rank >= 0 && rank <= ((phase_max/2)-1)){
			MPI_Status status;
			sender = rank | mask;
                        printf("[Reduction : Receiving] : Phase is %d Sender is %d and Recepient is %d\n", phase,sender,rank);

                        MPI_Probe(sender, 1, MPI_COMM_WORLD, &status);

                        MPI_Get_count(&status, MPI_INT, &recv_size);

			recv_arr = (int*)malloc(sizeof(int)*recv_size);
			MPI_Recv(recv_arr,recv_size,MPI_INT,sender,1,MPI_COMM_WORLD,&stat);

			print_g(recv_arr,recv_size,0,0,phase);
			pres_size_tmp = pres_size + recv_size;
			pres_tmp = (int*)malloc(sizeof(int)*pres_size_tmp);
			copy_arr(pres,pres_tmp,pres_size,0,0);
			copy_arr(recv_arr,pres_tmp,recv_size,0,pres_size);
			free(pres);
			free(recv_arr);
	
			sor_arr = (int*)malloc(sizeof(int)*pres_size_tmp);
			merge(pres_tmp,sor_arr,0,pres_size-1,pres_size_tmp-1);
			free(sor_arr);
			pres_size = pres_size_tmp;
			pres = (int*)malloc(sizeof(int)*pres_size);
			copy_arr(pres_tmp,pres,pres_size,0,0);
			free(pres_tmp);
			//display(pres,pres_size);
		}
		mask = mask >> 1;
	}

	if(rank == 0){
		time = clock() - time;
		display(pres,pres_size);
		printf("\nTime taken is %f\n",((double)time)/CLOCKS_PER_SEC);
	}

	MPI_Finalize();
}
