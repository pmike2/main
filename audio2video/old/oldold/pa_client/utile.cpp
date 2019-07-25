
#include "utile.h"

using namespace std;


/*
void write2log(char * str){
#ifdef __APPLE__
	cout << str << endl;
#else
	ofstream myfile;
	myfile.open(CH_LOG, ios::out | ios::app);
	myfile << str;
	myfile << "\n";
	myfile.close();
#endif
}
*/

// renvoie un double aléatoire entre x0 et x1
double rand_double(double x0, double x1)
{
	if (x1> x0)
		return x0+ (x1- x0)* (double)(rand()% 10000)/ 10000;
	else
		return x1+ (x0- x1)* (double)(rand()% 10000)/ 10000;
}


// renvoie un int aléatoire entre x0 et x1 compris
int rand_int(int x0, int x1)
{
	if (x1> x0)
		return x0+ rand()% (x1- x0+ 1);
	else
		return x1+ rand()% (x0- x1+ 1);
}


// Remplissage d'une chaine de caractères avec le contenu du fichier d'un shader
char* load_source(const char *filename)
{
    char *src = NULL;   /* code source de notre shader */
    FILE *fp = NULL;    /* fichier */
    long size;          /* taille du fichier */
    long i;             /* compteur */
	
	char filename_complet[512];

	absolute_path(filename, filename_complet);
	
    /* on ouvre le fichier */
    fp = fopen(filename_complet, "r");
    /* on verifie si l'ouverture a echoue */
    if (fp == NULL)
    {
        cout << "impossible d'ouvrir le fichier " << filename << endl;
        return NULL;
    }
	
    /* on recupere la longueur du fichier */
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
	
    /* on se replace au debut du fichier */
    rewind(fp);
	
    /* on alloue de la memoire pour y placer notre code source */
    src = (char *)malloc(size+1); /* +1 pour le caractere de fin de chaine '\0' */
    if (src == NULL)
    {
        fclose(fp);
        cout << "erreur d'allocation de memoire" << endl;
        return NULL;
    }
	
    /* lecture du fichier */
    for(i=0; i<size; i++)
        src[i] = fgetc(fp);
	
    /* on place le dernier caractere a '\0' */
    src[size] = '\0';
	
    fclose(fp);
	
    return src;
}


// assignation, compilation et verification que la compilation s'est bien faite
GLuint load_shader(GLenum type, const char *filename)
{
    GLuint shader = 0;
    GLsizei logsize = 0;
    GLint compile_status = GL_TRUE;
    char *log = NULL;
    char *src = NULL;
	
    /* creation d'un shader de sommet */
    shader = glCreateShader(type);
    if (shader == 0)
    {
        cout << "impossible de creer le shader" << endl;
        return 0;
    }
	
    /* chargement du code source */
    src = load_source(filename);
    if (src == NULL)
    {
		cout << "Le fichier de shader n'existe pas" << endl;
        glDeleteShader(shader);
        return 0;
    }
	
    /* assignation du code source */
    glShaderSource(shader, 1, (const GLchar**)&src, NULL);
	
    /* compilation du shader */
    glCompileShader(shader);
	
    /* liberation de la memoire du code source */
    free(src);
    src = NULL;
	
    /* verification du succes de la compilation */
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
    if (compile_status != GL_TRUE)
    {
        /* erreur a la compilation recuperation du log d'erreur */
		
        /* on recupere la taille du message d'erreur */
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logsize);
		
        /* on alloue un espace memoire dans lequel OpenGL ecrira le message */
        log = (char *)malloc(logsize + 1);
        if (log == NULL)
        {
            cout <<  "impossible d'allouer de la memoire" << endl;
            return 0;
        }
        /* initialisation du contenu */
        memset(log, '\0', logsize + 1);
		
        glGetShaderInfoLog(shader, logsize, &logsize, log);
        cout << "impossible de compiler le shader" << filename << " : " << log << endl;
		
        /* ne pas oublier de liberer la memoire et notre shader */
        free(log);
        glDeleteShader(shader);
		
        return 0;
    }
	
    return shader;
}


// Verif que le link des shaders de prog s'est bien déroulé
void check_gl_program(GLuint prog)
{
	GLint status;

	glGetProgramiv(prog, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		int loglen;
		char logbuffer[1000];
		glGetProgramInfoLog(prog, sizeof(logbuffer), &loglen, logbuffer);
		cout << "OpenGL Program Linker Error " << loglen << " ; " << logbuffer << endl;
	}
	else {
		int loglen;
		char logbuffer[1000];
		glGetProgramInfoLog(prog, sizeof(logbuffer), &loglen, logbuffer);
		if (loglen > 0) {
			cout << "OpenGL Program Linker OK " << loglen << " ; " << logbuffer << endl;
		}
		glValidateProgram(prog);
		glGetProgramInfoLog(prog, sizeof(logbuffer), &loglen, logbuffer);
		if (loglen > 0) {
			cout << "OpenGL Program Validation results " << loglen << " ; " << logbuffer << endl;
		}
	}
}


void calculate_normal(float *coord1, float *coord2, float *coord3, float *norm)
{
	/* calculate Vector1 and Vector2 */
	float va[3], vb[3], vr[3], val;
	va[0]= coord1[0]- coord2[0];
	va[1]= coord1[1]- coord2[1];
	va[2]= coord1[2]- coord2[2];
	
	vb[0]= coord1[0]- coord3[0];
	vb[1]= coord1[1]- coord3[1];
	vb[2]= coord1[2]- coord3[2];
	
	/* cross product */
	vr[0]= va[1]* vb[2]- vb[1]* va[2];
	vr[1]= vb[0]* va[2]- va[0]* vb[2];
	vr[2]= va[0]* vb[1]- vb[0]* va[1];
	
	/* normalization factor */
	val= sqrt(vr[0]* vr[0]+ vr[1]* vr[1]+ vr[2]* vr[2]);
	
	norm[0]= vr[0]/ val;
	norm[1]= vr[1]/ val;
	norm[2]= vr[2]/ val;
}


void absolute_path(const char * rel_path, char * abs_path) {
#ifdef __APPLE__
	CFBundleRef mainBundle = CFBundleGetMainBundle();
	CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
	char path[PATH_MAX];
	if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX))
		cout << "Erreur lors de la recherche du chemin courant" << endl;
	CFRelease(resourcesURL);
	
	string s1(path);
	string s2(rel_path);
	string concat= s1+ "/"+ s2;
	strcpy(abs_path, concat.c_str());
#else
	strcpy(abs_path, rel_path);
#endif
}


unsigned int diff_time_ms(struct timeval * after, struct timeval * before) {
	struct timeval diff;
	timersub(after, before, &diff);
	unsigned long i= diff.tv_sec* 1e6+ diff.tv_usec;
	return i/ 1000;
}


unsigned int diff_time_ms_from_now(struct timeval * begin) {
	struct timeval now;
	gettimeofday(&now, NULL);
	return diff_time_ms(&now, begin);
}


vector<string> list_files(string ch_dir, string ext) {
	vector<string> res;
	DIR *d;
	struct dirent *dir;
	d = opendir(ch_dir.c_str());
	if (d) {
		while ((dir = readdir(d)) != NULL) {
			string f= string(dir->d_name);
			if ((ext== "") || (f.substr(f.find_last_of(".")+ 1)== ext))
				//res.push_back(f.substr(0, f.find_last_of(".")));
				res.push_back(f);
		}
		closedir(d);
	}
	
	return res;
}


glm::vec3 sum_over_e(glm::vec3* e, glm::vec3* e_prime, int& i) {
    int k = 0;
    glm::vec3 result;
	//cout << "i=" << i << endl;
    while (k < i)
    {
    	//cout << "k=" << k << endl;
        float e_prime_k_squared = glm::dot(e_prime[k], e_prime[k]);
        result += (glm::dot(e[i], e_prime[k]) / e_prime_k_squared) * e_prime[k];
        k++;
    }
    //float * debug= glm::value_ptr(result);
	//cout << debug[0] << ";" << debug[1] << ";" << debug[2] << endl;
    return result;
}


void gram_schmidt(float * mat) {
	glm::vec3 e[] = {
		glm::vec3(mat[0], mat[1], mat[2]),
		glm::vec3(mat[4], mat[5], mat[6]),
		glm::vec3(mat[8], mat[9], mat[10])
	};
	glm::vec3 e_prime[3];
	e_prime[0]= e[0];
	int i= 0;

	do {
		e_prime[i]= e[i]- sum_over_e(e, e_prime, i);
		i++;
	} while (i< 3);
	
	for (i=0; i<3; i++)
		e_prime[i]= glm::normalize(e_prime[i]);
	
	float * pt;
	pt= glm::value_ptr(e_prime[0]);
	memcpy(mat, pt, sizeof(float) * 3);
	pt= glm::value_ptr(e_prime[1]);
	memcpy(mat+ 4, pt, sizeof(float) * 3);
	pt= glm::value_ptr(e_prime[2]);
	memcpy(mat+ 8, pt, sizeof(float) * 3);
}
