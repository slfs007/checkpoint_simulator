
#include<stdio.h>
#include<stdlib.h>
#include<time.h>

void generate_random_file( int unit_num, FILE *file);

int main ( int argc, char *argv[])
{
    FILE *rfg_file;
    int unit_num;
    
    if ( argc != 3)
    {
        perror("usage: ./rf_generator [file name]  [unit number]");
        exit(1);
    }
    
    if (  NULL == (rfg_file = fopen( argv[1], "w")))
    {
        perror("fopen error");
    }
    
    unit_num = atoi(argv[2]);
    printf("unit num:%d\n",unit_num);
    
    generate_random_file(  unit_num,rfg_file);
    fclose(rfg_file);
    exit(0);
}
void generate_random_file(int unit_num, FILE *file)
{
    int i;
    int rand_num;
    
    srand(time(NULL));
    //test pull request
    for ( i = 0; i < unit_num;i++)
    {
        rand_num = rand();
        fwrite( &rand_num, sizeof(int), 1, file);
    }
    fflush(file);
}
