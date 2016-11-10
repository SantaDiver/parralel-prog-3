/* C Example */
#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <vector>
#include <fstream>
 
using std::vector;
using std::cout;
using std::ofstream;
using std::endl;
 
typedef unsigned int uint;

int main (int argc, char* argv[])
{
	MPI_Init (&argc, &argv);      /* starts MPI */  
	
  if (argc != 4) return -1;
  
  int rank;
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);        /* get current process id */
  
  int size;
  MPI_Comm_size (MPI_COMM_WORLD, &size);        /* get number of processes */
  
  int a = atoi(argv[1]);
  int b = atoi(argv[2]);
  
  char out[256];
  strcpy(out, argv[3]);
  
  int bsqr = (int)ceil(sqrt(b));
  
  vector<bool> primeIndecators (b-a+1, true);
  primeIndecators[0] = false;
  primeIndecators[1] = false;
  
  for (uint i=2; i<=bsqr; ++i)
  {
    if (primeIndecators[i])
      for (uint j=i+i; j<=bsqr; j+=i)
      {
        primeIndecators[j] = false;
      }
  }
  
  double begin=MPI_Wtime();
  
  int left = bsqr + (b-bsqr)*rank/size + 1;
	int right = bsqr + (b-bsqr)*(rank+1)/size;
  
  for (uint i=left; i<=right; ++i)
	{
		for (uint j=2; j<=bsqr; ++j)
		{
			if ((primeIndecators[i]) && (i%j == 0))
			{
				primeIndecators[i]=false;
				break;
			}
		}
	}
	
	double end=MPI_Wtime();
  
  if (rank == 0) 
  {
    vector<double> times(size);
    times[0] = end-begin;
    
    MPI_Status stat;
    
    for (uint i=1; i<size; ++i)
		{
			left = bsqr + (b-bsqr)*i/size + 1;
			right = bsqr + (b-bsqr)*(i+1)/size;
			//cout << left << " " << right << endl;
			int length = right - left + 1;
			//bool recieve[length];
			bool *recieve = new bool[length];
			MPI_Recv(recieve, length, MPI_C_BOOL, i, 1, MPI_COMM_WORLD, &stat);
			
			for (uint j=0; j<length; ++j)
			{
				primeIndecators[left+j] = recieve[j];
			}
			
			delete[] recieve;
		}
    
		for (uint i=1; i<size; ++i)
		{	
			MPI_Recv(&times[i], 1, MPI_DOUBLE, i, 2, MPI_COMM_WORLD, &stat);
		}
		
		double maxtime = times[0];
		double sumtime = times[0];
		for (uint i=1; i<size; ++i)
		{
			sumtime+=times[i];
			if (times[i] > maxtime) maxtime=times[i];
		}
		
		cout << size << " " << sumtime << " " << maxtime << endl;
		
		ofstream file;
		file.open(out,std::ofstream::app);
		
		int num=0;
		for (uint i=a; i<=b; ++i)
		{
		  if (primeIndecators[i]) ++num;
		}
		file << num << " ";
		
		for (uint i=a; i<=b; ++i)
		{
			if (primeIndecators[i]) 
			{
				file << i << " ";
				
				// for (uint j=2; j<i; ++j)
				// 	if (i%j == 0)
				// 	{
				// 		cout << "(_!_) - " << i << endl;
				// 		break;
				// 	}
			}
		}
		file.close();
  }
  else
  {
    int length = right-left+1;
    
    bool *part = new bool[length];
    //bool part[length];
    
		std::copy(primeIndecators.begin()+left, primeIndecators.begin()+right+1, part);
		
    MPI_Send(part, length, MPI_C_BOOL, 0, 1, MPI_COMM_WORLD);
    
    delete[] part;
    
    double t = end-begin;
		MPI_Send(&t, 1, MPI_DOUBLE, 0, 2, MPI_COMM_WORLD);
  }
  
  MPI_Finalize();
  return 0;
}