#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <unistd.h> /* for fork */
#include <sys/wait.h>
#include <dirent.h> /*Directory Access*/
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h> /*Path max*/
#include <fcntl.h>
#ifndef PATH_MAX
#define PATH_MAX 255
#endif
#define DIR_MAX 100
#define FIFO_PERM S_IRUSR|S_IWUSR
#define OUTPUT "log.txt"

int readFromFile( const char * filename , const char *string , const char * filePathName);
int readFromDirectory(  char * path , const char * string , char*argv);
int size(const char * filename);
int isDirectory(const char * path);
int string_length(const char * str);
void copystr(char* destination,  const char* source);
char* concat(char* destination, const char* source);
int compare( const char * str1, const char * str2);

void red(){
  printf("\033[0;31m");
}

void green(){
  printf("\033[0;32m");
}

void blue(){
  printf("\033[0;34m");
}

void purple(){
  printf("\033[0;35m");
}

void resetColor(){
  printf("\033[0m");
}

int main(int argc , char *argv []){

	FILE * opf;
	
	int total;
	
	if(argc != 3){ /* Usage status: istenen argüman girilmeyince*/
		
		printf("Usage : ./exampleProgram stringToSearch DirectoryToSearch\n");
		
		return 1;
	}
	
	total = readFromDirectory( argv[2] , argv[1]  , argv[2]); 
	
	opf = fopen(OUTPUT , "a");
	
	fprintf(opf, "\n\n\n %d %s kez bulundu.\n", total , argv[1]);
	red();
	printf( "\n\n\n %d %s bulundu.\n", total , argv[1]);
	resetColor();
	printf("\n ************************");
	fclose(opf);
	
	return 0;
}

int readFromDirectory( char * path , const char * string , char*argv){
	
	DIR * dirp; /*Dosya acilimi*/
	FILE * opf; /* Log dosyasini acmak icin*/
	struct dirent *direntp; /*dosya okuma*/
	char *token , new_path[PATH_MAX];
	int  count = 0 , status=0 , total=0,openFifo,countTempDir=0 , result=0,totalDir=0 ;
	int i=0 , t=0 , l=0;
	int dirCount=0 , fileCount=0; 
	char fifoArr[DIR_MAX][PATH_MAX] , temp_path[PATH_MAX] , tempArr[100] , direc_name[100] , fifoName[DIR_MAX];
	int fd[2] , openFifo2;
	pid_t pid; /* fork islemi icin*/
    
	for(l=0;l<DIR_MAX;++l){
		fifoArr[l][PATH_MAX] ='\0';
	}
	opf = fopen(OUTPUT ,"a"); /* log.txt acilir.*/
 	
	if((dirp = opendir(path)) == NULL){  /*Dosya acilidginin kontrolu*/
		
		fprintf(stderr , "Directory not opened\n");
		
		exit(1);
	}
	else{
		
		copystr(temp_path , path);
		
		while((direntp = readdir(dirp)) != NULL){   /* Dosyayi okur Directorylerin icine girerek directory kalmayana kadar doner*/

	
			
			copystr(temp_path , path);

			concat(temp_path , "/"); 
			
			if(compare(direntp->d_name,".") != 0 && compare(direntp->d_name,"..") != 0  ){/* klasorun olup olmadiginin= dogrulugunun kontrolu*/
					
				concat(temp_path , direntp->d_name);
					
				if(isDirectory(temp_path) == 0){

					++dirCount;
					
				}
				else if(isDirectory(temp_path) != 0 ){ 
					
					++fileCount;
					
				}
			}
		}
	}
	/*Directory basa doner ve ayni islem tekrarlanir*/
	rewinddir(dirp);
	
	if((dirp = opendir(path)) == NULL){ 
		
		fprintf(stderr , "Directory not opened\n");
		
		exit(1);
	}
	else{
			
		if((dirp = opendir(path)) == NULL){ 
		
			fprintf(stderr , "Directory not openedddd\n");
			
			exit(1);
		}
		else{
			
			copystr(temp_path , path);
			
			while((direntp = readdir(dirp)) != NULL){ 
			 	
				copystr(temp_path , path);

				concat(temp_path , "/"); 
				
				if(compare(direntp->d_name,".") != 0 && compare(direntp->d_name,"..") != 0  ){
						
					concat(temp_path , direntp->d_name);
						
					if(isDirectory(temp_path) == 0){
						
						sprintf(tempArr , "%d%s" , getpid(), direntp->d_name);
						
						copystr(fifoArr[t] , tempArr );
						
						++t;
						
						if(mkfifo(tempArr , FIFO_PERM) == -1){
							
							fprintf(stderr, "FIFO not created\n");
							
							exit(1);
						}
					}
				}
			}
		}
		
		rewinddir(dirp);

		if(fileCount > 0){
		
			if(pipe(fd) == -1){/*Pipe control*/
			
				fprintf(stderr, "Pipe not created\n" );

				exit(1);
			
			}
		}
		
		copystr(new_path , path);
		
		while((direntp = readdir(dirp)) != NULL){ 

			copystr(new_path , path);

			concat(new_path , "/"); 
			
			if(compare(direntp->d_name,".") != 0 && compare(direntp->d_name,"..") != 0  ){
   				
   				concat(new_path , direntp->d_name);
   				
   				
   				if(isDirectory(new_path) == 0){

					if((pid = fork()) == -1){ 
						
						fprintf(stderr, "Fork failed.\n" );
						
						exit(1);
					}

					else if( pid == 0){ /*child process*/

						
						readFromDirectory( new_path , string , argv);
						
						exit(1);
					}
					
				}
				else if(isDirectory(new_path) != 0 ){ 
					
				
					if((pid = fork()) == -1){ 
						
						fprintf(stderr, "Fork failed.\n" );
						
						exit(1);
					}
					else if( pid == 0){ /*child process*/
						

						count = readFromFile(new_path , string , direntp->d_name);

						/*Child process filelarin icine girer ve aranan stringi okur kac tane oldugunu belirler*/
						write(fd[1] , &count , sizeof(count));

						/*Dosyadan okunan hedef stringi pipe a yazar*/

						exit(1);

					}
					else if(pid>0){ /*Parent process okuma yapar*/
							
						read(fd[0] , &count , sizeof(count));
						/*Parent process pipedaki veriyi okur ve totale ekler.Her defasinda yaparak 
						ayni directory icindeki toplam hedef stringi hesaplanir.*/
						total += count;	
						
					}
				}
			}
		}
	}
	
	
	
	for(i=0;i<dirCount ;++i){ 

		/*Fifo`ya yazilan veri okunur.*/
		copystr(fifoName , fifoArr[i]);
		
		if((openFifo=open(fifoName , O_RDONLY)) !=-1){
			
			read(openFifo , &countTempDir , sizeof(countTempDir));
			
			close(openFifo);
			
			totalDir += countTempDir; 
			
			unlink(fifoName); /*fifo kaldirilir*/ 
			
		}
	}
	
	result =  total + totalDir ; 

	/*Aranan string sayisi bulunulan dosya icin ve toplam sayisi hesaplanir.*/
	
	while(wait(&status) != -1); /*Parent , childlari bekler.*/
	
	if(strcmp( path , argv )!=0){ 

		token = strtok(path, "/");

        while (token != NULL) {
            copystr(direc_name, token);

            token = strtok(NULL, "/");

        }
		sprintf(fifoName , "%d%s" ,getppid(),direc_name); /*Fifo Tanimi*/
	    
	    if((openFifo2 = open(fifoName, O_WRONLY)) !=-1){
	    	
	    	write(openFifo2, &result, sizeof(result));
	    
	    	close(openFifo2);
	    }
	}

	closedir(dirp); /*dizini kapatir*/

	fclose(opf); /*DOsyayi Kapatir*/
    
    return result;
}
		
int readFromFile( const char * filename , const char *string , const char * filePathName){	/*Programin asil islem yapacagi fonksiyondur.Dosyadan alip arraya attigimiz karakterleri string ile karsılastiririz. */


	FILE * inf;
	FILE * opf;

	int string_size = 0 ; /* Argumanda verilen stringin boyutu*/
	int counter = 0 , row = 1 , column = 1 ; /* Satir - sutun belirlenir*/
 	int flag = 0; /*Durum kosuluna göre flag tutulur.*/
 	int k=0, tempCount=0;
	int t=0; /* karakter ilerlemek için kullanilan degisken*/
	int z=0;
    int i=0;
    int j=1;
	char *array;
	int array_size=0; /*Malloc ile dinamik yer ayirmak icin kullanilacak size degiskeni*/

	string_size = string_length(string); /* String boyutu alinir.*/

	array_size = size(filename);
	
	array = (char*)malloc((sizeof(char)*array_size)+1); 
	array[array_size] = '\0';
	
	inf = fopen(filename , "r");
	opf = fopen(OUTPUT , "a+");

	for(z=0; z < array_size ; ++z){ /* Dosyadan okuyup array`e atilir*/
		
		fscanf(inf , "%c" , &array[z]);
	}
	
	for(i=0 ; i < array_size; ++i){
		
		flag = 0;
		
		tempCount =0;
		
		if(array[i] == string[0]){
		
			k = column;
		
			t =0;
			
			for(j=1; j< string_size && flag== 0 ; ++j){
			
				if(array[i+j+t] == ' ' || array[i+j+t] == '\t' || array[i+j+t] == '\n'){

					for(; array[i+j+t] == ' ' || array[i+j+t] == '\t' || array[i+j+t] == '\n' ; ++t);
				}

				if(array[i+j+t] ==  string[j])
				
					++tempCount;
			
					if(tempCount == string_size-1){
			
						flag = 1;
						
						counter ++;
						
						fprintf(opf , " %s : [%d , %d] %s first character is found.\n" , filePathName , row , k , string);
					
					}
			}
			column++;
		}
		else{
			
			if(array[i] == ' ' || array[i] == '\t')
			
				column++;
			
			else if(array[i] == '\n'){ 
			
				row++;
			
				column = 1;
			
			}
			else
				column++;
		}

	}
	if(counter == 0){
		blue();
		printf("\n %s dosyasinda %s bulunamadi. \n" , filePathName , string );
		resetColor();
	}
	else
		green();
		printf("\n %s dosyasinda %d adet %s bulundu.\n" ,filePathName , counter , string);
		resetColor();
	
	fclose(inf);
	fclose(opf);

	free(array);
	return counter;
}
/*Verilen pathin directory olup olmadigi kontrolu*/
int isDirectory(const char * path){
	struct stat stat_array;

	if(stat(path , &stat_array) == -1 ){
		return 0;
	}
	else{
		return S_ISREG(stat_array.st_mode);
	}
}
/*Bu fonksiyon bir stringi diger stringe ekleme yapar. */
char* concat(char* destination, const char* source){
	int i,j;
	
	
	for(i=0;destination[i] !='\0';i++);
	for(j=0;source[j] !='\0'; j++){
		destination[j+i] =source[j];
	}
	destination[j+i]='\0';
	
	return destination;
}
/*Iki string arasinda karsilastirma yapar.*/
int compare( const char * str1, const char * str2){
	int i,j;
	for(i=0;str1[i] !='\0';i++){
		for(j=0;str2[j] !='\0'; j++){
			if(i==j){
				
				if(str1[i]>str2[j]){
					return -1;
				}
				else if(str1[i]<str2[j]){
					return 1;
				}				
			}
		}
	}
	
	return 0;
}
/*stringi kopyalayip baska stringe ekler.*/
void copystr(char* destination, const char* source){
	
	int i;
	
	for(i=0;source[i] !='\0';i++){
	
		destination[i]=source[i];
	
	}
	
	destination[i]='\0';

}

int size( const char * filename){

	int count = 0;
	FILE *input_file;
	
	input_file = fopen(filename,"r"); 
	
	if(input_file == NULL){ 
		
		perror("Could not open file.\n"); // Dosya acilamaz ise hata verir
		
		exit(1);
	}

	fseek(input_file, 0, 2); // Dosya boyutunu doner
	count = ftell(input_file);
	
	fclose(input_file);

	return count;
}

int string_length(const char * str){

	int i=0;

	for(;str[i] != '\0' ; ++i);

	return i; //string uzunlugunu doner
}