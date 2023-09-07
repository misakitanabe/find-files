extern int fd[2];

void writeToPipe(char *paths);
void find1(char input[256], int inLength, char *flagF, char *flagS);
int recursive1(char *paths, char *text, char *fileEnd, char *direct, int found, int option);
void find2(char input[256], int inLength, char *flagF);
int recursive3(char *paths, char *text, char *direct, int found, int option);
void find3(char input[256], int inLength, char *flagS);
char* fileToBuffer(char* filename);
void find4(char input[256], int inLength);
void find5(char input[256], int inLength, char *flagS);
int recursive5(char *paths, char *filename, char *direct, int wrote, int option);
void find6(char input[256], int inLength);
int findFile(char *filename, int option, char *paths, int wrote);