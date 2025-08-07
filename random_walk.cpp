#include <iostream>
#include <cstdlib> // For atoi, rand, srand
#include <ctime>   // For time
#include <mpi.h>

void walker_process();
void controller_process();

int domain_size;
int max_steps;
int world_rank;
int world_size;

int main(int argc, char **argv)
{
    // Initialize the MPI environment
    MPI_Init(&argc, &argv);

    // Get the number of processes and the rank of this process
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    if (argc != 3)
    {
        if (world_rank == 0)
        {
            std::cerr << "Usage: mpirun -np <p> " << argv[0] << " <domain_size> <max_steps>" << std::endl;
        }
        MPI_Finalize();
        return 1;
    }

    domain_size = atoi(argv[1]);
    max_steps = atoi(argv[2]);

    if (world_rank == 0)
    {
        // Rank 0 is the controller
        controller_process();
    }
    else
    {
        // All other ranks are walkers
        walker_process();
    }

    // Finalize the MPI environment
    MPI_Finalize();
    return 0;
}

void walker_process()
{
     srand(time(NULL) + world_rank); // Different random seed per walker

    int position = 0;   // Start at position 0
    int steps = 0;

    while (steps < max_steps) {
        // Step randomly: -1 (left) or +1 (right)
        int move = (rand() % 2 == 0) ? -1 : 1;
        position += move;
        steps++;

        // Check if walker is out of bounds
        if (position < -domain_size || position > domain_size) {
            // Print required message
            std::cout << "Rank " << world_rank << ": Walker finished in " 
                      << steps << " steps." << std::endl;

            // Send completion message to controller
            MPI_Send(&steps, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
            return;
        }
    }

    // If max_steps reached (safety limit)
    std::cout << "Rank " << world_rank << ": Walker finished in " 
              << steps << " steps (max steps reached)." << std::endl;
    MPI_Send(&steps, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

}

void controller_process()
{
        int num_walkers = world_size - 1;  // Exclude controller
    MPI_Status status;

    for (int i = 0; i < num_walkers; i++) {
        int steps;
        // Receive from any walker that finishes
        MPI_Recv(&steps, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        int walker_rank = status.MPI_SOURCE;

        std::cout << "Controller: Walker " << walker_rank 
                  << " finished in " << steps << " steps." << std::endl;
    }

    std::cout << "Controller: All " << num_walkers << " walkers have finished." << std::endl;

}
