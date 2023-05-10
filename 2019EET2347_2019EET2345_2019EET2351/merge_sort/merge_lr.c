/* Compile - mpicc -o merge_lr merge_lr.c 
 * Execute - mpirun -np 8  --tag-output merge_lr */
// 8 = no. of processes
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#define BCAST "Broadcast"
#define REDUC "Reduction"
#define SEND "sending"
#define RECV "Receiving"

void display(int *a, int r, int size);

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


void copy_arr(int *master, int *slave, int length, int start_m, int start_s){

        int i = 0;
        int j = start_s;

        //slave = (int *)malloc(sizeof(int)*length);
        for (i=start_m; i < (start_m+length);i++){
                slave[j] = master[i];
                j++;
        }
}


void create_sarr(int *master, int *slave, int length, int start){

	int i = 0;
	int j = 0;
	
	//slave = (int *)malloc(sizeof(int)*length);
	for (i=start; i < (start+length);i++){
		slave[j] = master[i];
		j++;
	}
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
	int arr_size = 5;
	int sarr_size;
	int rec_size;
	int extra = 0;
	int *send_arr = NULL;
	int *rec_arr = NULL;
	clock_t time;

	MPI_Init(NULL, NULL); // object initialization

	MPI_Comm_size(MPI_COMM_WORLD, &world_size); // world_size shayad no. of process hai

	MPI_Comm_rank(MPI_COMM_WORLD, &rank); // process ka number hoga (0 to 7 taak leke rank honge)
		// linear ring sirf even no. pe chalta hai...yaha pe...but odd pe bhi chalna chahiye in general
		// but ye code odd no. of processes ko handel ni karsakta
        if (world_size%2 != 0 || world_size == 2){
                printf("Invalid number of processors.\n");
                exit(1);
        }

	//display(arr,rank);
	//arr_size = (sizeof(arr)/sizeof(arr[0]));
	// 0th number ka processor is master process
	if (rank == 0) {
		arr = (int*)malloc(sizeof(int)*arr_size); // arr_size = 5 hai idhar
		creat_ip_arr(arr,arr_size);
		time = clock();
	}
	// if arr_size > world_size tabhi 0th process distribute karega

	size = arr_size/world_size; // 17 / 8 then wo ek extra input kisi ek processor ko bhi handel karna padega
	extra = arr_size%world_size;

	//printf("arr size %d, size %d, extra %d arg is %s\n",arr_size, size, extra);	
	//sub_array = malloc(size * sizeof(int));
	//MPI_Scatter(arr, size, MPI_INT, sub_array, size, MPI_INT, 0, MPI_COMM_WORLD);


	if (rank == 0){
		int *rec_1 = NULL;
		int *rec_2 = NULL;
		int pos = 0;
		int mid = 0;
		int total_1 = 0;
		int total_2 = 0;
		int total = 0;
		MPI_Status stat;
		MPI_Status status;
		int *sor_arr = NULL;
		int *sub_arr = NULL;
		int *merge_1 = NULL;
		int *merge_2 = NULL;
		/*sub_arr = (int *)malloc(sizeof(int)*size);
		create_sarr(arr,sub_arr,size,pos);
		pos = pos + size;
		MPI_Send(&pos,1,MPI_INT,1,1,MPI_COMM_WORLD);
		pos = arr_size - size;
		MPI_Send(&pos,1,MPI_INT,world_size-1,1,MPI_COMM_WORLD);*/
		sub_arr = (int *)malloc(sizeof(int)*size); // size = 2 (16 and 8 example)
		copy_arr(arr,sub_arr,size,0,0);
		sarr_size = (((world_size/2) - rank) * size) + extra;// (8/2 - 0) * 2 = 8 (upper arr send size)
		send_arr = (int *)malloc(sizeof(int) * (sarr_size));
		copy_arr(arr,send_arr,sarr_size,size,0);// 2 se 9 total 8
		//display(sub_arr,rank,(sarr_size+1));
		print_g(send_arr,sarr_size,1,1);
		MPI_Send(send_arr,sarr_size,MPI_INT,1,1,MPI_COMM_WORLD); // 1 is indx of processor and ignore another 1(tag)
		free(send_arr); // send(1) completed here
		sarr_size = ((world_size - ((world_size/2)+1)) * size); // 6 elemts ko neeche waale side pe bhejna hai
		send_arr = (int *)malloc(sizeof(int) * (sarr_size));
		copy_arr(arr,send_arr,sarr_size,arr_size-sarr_size,0);
		//display(sub_arr,rank,(sarr_size+1));
		print_g(send_arr,sarr_size,1,1);
		MPI_Send(send_arr,sarr_size,MPI_INT,world_size-1,1,MPI_COMM_WORLD); // 7th processor ko bhej rahe hai
		free(send_arr); // k == 0 over here


      		//display(sub_arr,rank); 
      		if (size > 1){
                //if (0){
			sor_arr = (int *)malloc(sizeof(int)*size);
                        merge_sort(sub_arr,sor_arr,0,size-1);
			free(sor_arr);
                }

		printf("Rank is %d \n",rank);
		display(sub_arr,rank,size);

		/*total_1 = size + (((world_size/2) * size) + extra);
		rec_1 = (int *)malloc(sizeof(int) * (((world_size/2) * size) + extra));
                MPI_Recv(rec_1,(((world_size/2) * size) + extra),MPI_INT,1,1,MPI_COMM_WORLD,&stat);
		display(rec_1,rank,(((world_size/2) * size) + extra));*/
                MPI_Probe(1, 1, MPI_COMM_WORLD, &status);
                MPI_Get_count(&status, MPI_INT, &rec_size);
                rec_arr = (int *)malloc(sizeof(int)*rec_size);
                MPI_Recv(rec_arr,rec_size,MPI_INT,1,1,MPI_COMM_WORLD,&stat);
		print_g(rec_arr,rec_size,0,0);
		total_1 = size + rec_size;
		merge_1 = (int *)malloc(sizeof(int)*total_1);
		copy_arr(sub_arr, merge_1, size, 0,0);
		copy_arr(rec_arr, merge_1, rec_size, 0, size);
		sor_arr = (int *)malloc(sizeof(int)*total_1);
		merge(merge_1, sor_arr, 0,size-1, total_1-1);
		free(sor_arr);
		free(rec_arr);

		/*total_2 = (world_size - (world_size/2 + 1)) * size;
		rec_2 = (int *)malloc(sizeof(int) * ((world_size - (world_size/2 + 1)) * size));
		display(rec_2,rank,(world_size - (world_size/2 + 1)) * size);
                MPI_Recv(rec_2,total_2,MPI_INT,world_size-1,1,MPI_COMM_WORLD,&stat);*/
                MPI_Probe(world_size-1, 1, MPI_COMM_WORLD, &status);
                MPI_Get_count(&status, MPI_INT, &rec_size);
                rec_arr = (int *)malloc(sizeof(int)*rec_size);
                MPI_Recv(rec_arr,rec_size,MPI_INT,world_size-1,1,MPI_COMM_WORLD,&stat);
		print_g(rec_arr,rec_size,0,0);
		total_2 = rec_size;
		total = total_1 + total_2;
		merge_2 = (int *)malloc(sizeof(int) * total);
		copy_arr(merge_1, merge_2, total_1, 0, 0);
		copy_arr(rec_arr, merge_2, total_2, 0, total_1);
		sor_arr = (int *)malloc(sizeof(int)*total);
		merge(merge_2, sor_arr, 0, total_1-1, total-1);
		free(sor_arr);
		display(merge_2,rank,arr_size);

	} else if (rank == world_size/2){ // k == p / 2
        int *sor_arr_1 = NULL;
		int size_1 = 0;
		MPI_Status stat;
		int *sub_arr = NULL;
		MPI_Status status;

		/*MPI_Recv(&size_1,1,MPI_INT,(world_size/2)-1,1,MPI_COMM_WORLD,&stat);
		if (extra > 0){
			size = size + extra;
		}

		sub_arr = (int *)malloc(sizeof(int)*size);
		create_sarr(arr,sub_arr,size,size_1);*/

		MPI_Probe((world_size/2)-1, 1, MPI_COMM_WORLD, &status); // wait(p/2 - 1) blocking statement

		MPI_Get_count(&status, MPI_INT, &rec_size);// arr jo rec karaha hu uski size kitni hai (rec_size = 2)

		sub_arr = (int *)malloc(sizeof(int)*rec_size);
		MPI_Recv(sub_arr,rec_size,MPI_INT,(world_size/2)-1,1,MPI_COMM_WORLD,&stat); // 4th arg is sender ka id 5th ignore(i.e 3)
		print_g(sub_arr,rec_size,0,1);

      		//display(sub_arr,rank); 
                if (rec_size > 1){
                //if (0){
			sor_arr_1 = (int *)malloc(sizeof(int)*rec_size);
                        //merge_sort(sub_arr,sor_arr_1,size_1,(size_1+size)-1);
                        merge_sort(sub_arr,sor_arr_1,0,rec_size-1);
			free(sor_arr_1);
                }
		// and accumulation
		printf("Rank is %d\n",rank);
		display(sub_arr,rank,rec_size);
		print_g(sub_arr,rec_size,1,0);
                MPI_Send(sub_arr,rec_size,MPI_INT,(world_size/2)-1,1,MPI_COMM_WORLD); // 3 ko send kardiya


	} else if (rank == ((world_size/2)+1)){ // 5th wala 6th se rec karega
		int *sor_arr_2 = NULL;
		int size_2 = 0;
		MPI_Status stat;
		int *sub_arr = NULL;
                MPI_Status status;

		/*MPI_Recv(&size_2,1,MPI_INT,((world_size/2)+2) % world_size ,1,MPI_COMM_WORLD, &stat);
		sub_arr = (int *)malloc(sizeof(int)*size);
		create_sarr(arr,sub_arr,size,size_2);*/
      		//display(sub_arr,rank); 

		MPI_Probe(((world_size/2)+2) % world_size, 1, MPI_COMM_WORLD, &status);

		MPI_Get_count(&status, MPI_INT, &rec_size);

		sub_arr = (int *)malloc(sizeof(int)*rec_size);

		MPI_Recv(sub_arr,rec_size,MPI_INT,((world_size/2)+2) % world_size ,1,MPI_COMM_WORLD, &stat);// 5th 6th se recv kiya
		print_g(sub_arr,rec_size,0,1);

                if (size > 1){
                //if (0){
			printf("One more Rank is %d  size_1 %d\n",rank,size_2);
			sor_arr_2 = (int *)malloc(sizeof(int)*size);
                        //merge_sort(sub_arr,sor_arr_2,size_2,(size_2+size)-1);
                        merge_sort(sub_arr,sor_arr_2,0,size-1);
			free(sor_arr_2);
                }

		printf("Rank is %d\n",rank);
		display(sub_arr,rank,size);
		print_g(sub_arr,size,1,0);
		MPI_Send(sub_arr,size,MPI_INT,((world_size/2)+2) % world_size ,1,MPI_COMM_WORLD);


	} else if (rank > 0 && rank < world_size/2){
		int *rec_3 = NULL;
		int send_3 = 0;
		int size_3 = 0;
		int pos_1 = 0;
		int mid_1 = 0;
		int total_1 = 0;
		MPI_Status stat;
		int *sor_arr_3 = NULL;
		int *sub_arr = NULL;
		int *merge_1 = NULL;
		MPI_Status status;

		/*MPI_Recv(&size_3,1,MPI_INT,rank-1,1,MPI_COMM_WORLD,&stat);
		sub_arr = (int *)malloc(sizeof(int)*size);
		create_sarr(arr,sub_arr,size,size_3);
      		//display(sub_arr,rank); 
		pos_1 = size_3 + size;
		MPI_Send(&pos_1,1,MPI_INT,rank+1,1,MPI_COMM_WORLD);*/

		MPI_Probe(rank-1, 1, MPI_COMM_WORLD, &status); // wait(p-1) // blocking

		MPI_Get_count(&status, MPI_INT, &rec_size); // check for size

		rec_arr = (int *)malloc(sizeof(int)*rec_size);


		MPI_Recv(rec_arr,rec_size,MPI_INT,rank-1,1,MPI_COMM_WORLD,&stat); 
		print_g(rec_arr,rec_size,0,1);

		sub_arr = (int *)malloc(sizeof(int)*size);
		copy_arr(rec_arr,sub_arr,size,0,0); // size = 2
		//if (rank == 2) goto end;
		sarr_size = (((world_size/2) - rank) * size) + extra;
		send_arr = (int *)malloc(sizeof(int) * (sarr_size));
		copy_arr(rec_arr,send_arr,sarr_size,size,0);

		print_g(send_arr,sarr_size,1,1);
		MPI_Send(send_arr,sarr_size,MPI_INT,rank+1,1,MPI_COMM_WORLD);// send(p+1)
		free(send_arr);

                if (size > 1){ // execution 
                //if (0){
			sor_arr_3 = (int *)malloc(sizeof(int)*size);
                        merge_sort(sub_arr,sor_arr_3,0,size-1); // execution 
			free(sor_arr_3);
                }

		printf("Rank is %d\n",rank);
		display(sub_arr,rank,size);
		// and accumulation---------------------------------------------------
		/*total_1 = size + (((world_size/2 - rank) * size) + extra);
		rec_3 = (int *)malloc(sizeof(int) * (((world_size/2 - rank) * size) + extra));
		MPI_Recv(rec_3,(((world_size/2 - rank) * size) + extra),MPI_INT,rank+1,1,MPI_COMM_WORLD,&stat);
		display(rec_3,rank,(((world_size/2 - rank) * size) + extra));*/
                MPI_Probe(rank+1, 1, MPI_COMM_WORLD, &status);
                MPI_Get_count(&status, MPI_INT, &rec_size);
                rec_arr = (int *)malloc(sizeof(int)*rec_size);
                MPI_Recv(rec_arr,rec_size,MPI_INT,rank+1,1,MPI_COMM_WORLD,&stat); // rank + 1 se rec karna hai 
                print_g(rec_arr,rec_size,0,0);
		total_1 = size + rec_size;
		merge_1 = (int *)malloc(sizeof(int)*total_1);
		copy_arr(sub_arr, merge_1, size, 0,0);
		copy_arr(rec_arr, merge_1, rec_size, 0, size);
		sor_arr_3 = (int *)malloc(sizeof(int)*total_1);
		merge(merge_1, sor_arr_3, 0, size-1, total_1 - 1);
		free(sor_arr_3);
		free(rec_arr);
		
		print_g(merge_1,total_1,1,0);
                MPI_Send(merge_1,total_1,MPI_INT,rank-1,1,MPI_COMM_WORLD); // rank - 1 ko send kardiya


	} else if (rank > ((world_size/2)+1) && rank <= (world_size - 1)){
		int *rec_4 = NULL;
		int send_4 = 0;
		int size_4 = 0;
		int pos_2 = 0;
		int mid_2 = 0;
		int total_1 = 0;
		MPI_Status stat;
		int *sor_arr_4 = NULL;
		int *sub_arr = NULL;
		int *merge_1 = NULL;
		MPI_Status status;

		
		/*MPI_Recv(&size_4,1,MPI_INT,(rank+1)%world_size,1,MPI_COMM_WORLD,&stat);
		sub_arr = (int *)malloc(sizeof(int)*size);
		create_sarr(arr,sub_arr,size,size_4);
      		//display(sub_arr,rank); 
		pos_2 = size_4 - size;
		MPI_Send(&pos_2,1,MPI_INT,rank-1,1,MPI_COMM_WORLD);*/
		
		MPI_Probe((rank+1)%world_size, 1, MPI_COMM_WORLD, &status);

		MPI_Get_count(&status, MPI_INT, &rec_size);

		rec_arr = (int *)malloc(sizeof(int)*rec_size);

		MPI_Recv(rec_arr,rec_size,MPI_INT,(rank+1)%world_size,1,MPI_COMM_WORLD,&stat);
		print_g(rec_arr,rec_size,0,1);

		sub_arr = (int *)malloc(sizeof(int)*size);
		copy_arr(rec_arr,sub_arr,size,0,0);
		sarr_size = ((rank - ((world_size/2)+1)) * size);
		send_arr = (int *)malloc(sizeof(int) * (sarr_size));
		copy_arr(rec_arr,send_arr,sarr_size,size,0);


		print_g(send_arr,sarr_size,1,1);
		MPI_Send(send_arr,sarr_size,MPI_INT,rank-1,1,MPI_COMM_WORLD);
		free(send_arr);

		if (size > 1){ // atleast need 2 elements for sorting
                //if (0){
			sor_arr_4 = (int *)malloc(sizeof(int)*size);
			merge_sort(sub_arr,sor_arr_4,0,size-1);
			free(sor_arr_4);
		}

		printf("Rank is %d\n",rank);
		display(sub_arr,rank,size);
		/*total_1 = size + ((rank - ((world_size/2)+1)) * size);
		rec_4 = (int *)malloc(sizeof(int)*((rank - ((world_size/2)+1)) * size));
		MPI_Recv(rec_4,((rank - ((world_size/2)+1)) * size),MPI_INT,rank-1,1,MPI_COMM_WORLD,&stat);
		display(rec_4,rank,((rank - ((world_size/2)+1)) * size));*/
		// accumulation started
		MPI_Probe(rank-1, 1, MPI_COMM_WORLD, &status);
                MPI_Get_count(&status, MPI_INT, &rec_size);
                rec_arr = (int *)malloc(sizeof(int)*rec_size);
                MPI_Recv(rec_arr,rec_size,MPI_INT,rank-1,1,MPI_COMM_WORLD,&stat);
                print_g(rec_arr,rec_size,0,0);
		total_1 = size + rec_size;
		merge_1 = (int *)malloc(sizeof(int)*total_1);
		/*copy_arr(rec_4, merge_1, ((rank - ((world_size/2)+1)) * size), 0,0);
		copy_arr(sub_arr, merge_1, size, 0,((rank - ((world_size/2)+1)) * size));*/
		copy_arr(sub_arr, merge_1, size, 0,0);
                copy_arr(rec_arr, merge_1, rec_size, 0, size);
                sor_arr_4 = (int *)malloc(sizeof(int)*total_1);
                merge(merge_1,sor_arr_4,0,size-1, total_1-1);
                free(sor_arr_4);


		print_g(merge_1,total_1,1,0);
                MPI_Send(merge_1,total_1,MPI_INT,(rank+1)%world_size,1,MPI_COMM_WORLD);

	}

	//sleep(600);
	// Finalize the MPI environment.
	if (rank == 0){
		time = clock() - time;
		//display(merge_2,rank,arr_size);
		printf("\nTime taken is %f\n",((double)time)/CLOCKS_PER_SEC);
	}

	MPI_Finalize();
}
