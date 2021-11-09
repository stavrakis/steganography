#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_COMMENT_SIZE 500

struct linkedList {
	char *str;
	struct linkedList *next;
};
struct PPM {
	char *ppm_type;
	struct linkedList *comments;
	int w;
	int h;
	int maxColorValue;
	int *imageMatrix;
};

struct PPM * getPPM(FILE * fd) {
	char txt[MAX_COMMENT_SIZE];
	struct PPM *out = malloc(sizeof(struct PPM));
	fgets(txt,MAX_COMMENT_SIZE,fd);
	out->ppm_type = (char *) malloc(strlen(txt));
	strcpy(out->ppm_type,txt);

    struct linkedList *new_node;
	while(fgets(txt,MAX_COMMENT_SIZE,fd)) {
		if(txt[0] == '#') {
			new_node = (struct linkedList*) malloc(sizeof(struct linkedList));
			new_node->str = (char *) malloc(strlen(txt));
            new_node->next = NULL;
			strcpy(new_node->str, txt);
			if (out->comments == NULL) {
                out->comments = new_node;
			} else {
				
				struct linkedList *current_node = out->comments;
				while (current_node->next != NULL) {
					current_node = current_node->next;
				}
                current_node->next = new_node;
			}
			
		} else {
			sscanf(txt,"%d %d",&out->w,&out->h);
			fgets(txt,MAX_COMMENT_SIZE,fd);
			sscanf(txt,"%d",&out->maxColorValue);
			break;
		}
	}
	
	out->imageMatrix = (int*) malloc(3*out->w*out->h*sizeof(int));
	int i=0;
	
    for(i=0;fscanf(fd,"%d",&out->imageMatrix[i]) != EOF;i++);
    
	return out;
}

void showPPM (struct PPM *i) {
	printf("%s",i->ppm_type);
    
	struct linkedList *comment = i->comments;
	while (comment != NULL) {
		printf("%s",comment->str);
		comment = comment->next;
	}
    
	printf("%d %d\n",i->w,i->h);
	printf("%d\n",i->maxColorValue);
	
    
	int c;
    for(c=0;c<i->w*i->h*3;c+=3) {
        printf("%d %d %d\n", i->imageMatrix[c], i->imageMatrix[c+1], i->imageMatrix[c+2]);
    }
	
}

void savePPM (struct PPM *o, FILE *out) {
	fputs(o->ppm_type, out);
	struct linkedList *comment = (struct linkedList*) malloc(sizeof(struct linkedList));
	comment = o->comments;
	while (comment != NULL) {
		fprintf(out,"%s",comment->str);
		comment = comment->next;
	}
	free(comment);
	fprintf(out,"%d %d\n%d\n",o->w,o->h,o->maxColorValue);
	
	int i;
    for(i=0;i<3*o->w*o->h;i+=3){
        fprintf(out,"%d %d %d\n",o->imageMatrix[i],o->imageMatrix[i+1], o->imageMatrix[i+2]);
    }
}

struct PPM * encode (char *text, struct PPM *i) {
	struct PPM * img = (struct PPM*) malloc(sizeof(i));
	img->ppm_type = strdup(i->ppm_type);
	img->h = i->h;
	img->w = i->w;
	img->maxColorValue = i->maxColorValue;
	
	struct linkedList *orig_comment = i->comments;
    struct linkedList *new_comment = NULL;
    struct linkedList *prev_comment = NULL;
    
	while (orig_comment != NULL) {
		new_comment = (struct linkedList*) malloc(sizeof(struct linkedList));
        new_comment->str = strdup(orig_comment->str);
        if (img->comments == NULL) img->comments = new_comment;
        else prev_comment->next = new_comment;
        prev_comment = new_comment;
        orig_comment = orig_comment->next;
	}
	
	img->imageMatrix = (int*) malloc(sizeof(int)*i->h*i->w*3);
	memcpy(img->imageMatrix, i->imageMatrix, sizeof(int)*i->h*i->w*3);
	
    if (strlen(text) + 1 > (img->w*img->h) /3) {
        printf("Not enough space for text\n");
        printf("String length: %d, width: %d, height: %d, max string length possible: %d\n", strlen(text), img->w, img->h, (img->w*img->h)/3 - 1);
        return NULL;
    }
    
	int c,j;
    int current_pos=0, extra_pos=0;
    int max_width = i->h*i->w / strlen(text) / 3;
    
	int seconds = time(NULL);
	//printf("seed = %d\n", seconds);
	
    srand(5);
    
    for (c=0;c<strlen(text);c++) {
		//printf("flag %d, %d -> %d\n", current_pos*3, img->imageMatrix[current_pos*3], img->imageMatrix[current_pos*3] ^ 1);
        
        current_pos += rand() % (max_width + extra_pos);
        //printf("current_pos= %d\n", current_pos*3);
        extra_pos =  (max_width * (c+1)) - current_pos;
        
		img->imageMatrix[current_pos*3] ^= 1;
        
		for (j=1;j<=8;j++) {
			//printf("element %d %d -> ", current_pos*3+j, img->imageMatrix[current_pos*3+j]);
			img->imageMatrix[current_pos*3+j] = (img->imageMatrix[current_pos*3+j] & 254) | ((text[c] >> j-1) & 1);
			//printf("%d\n", img->imageMatrix[current_pos*3+j]);
		}
		current_pos += 3;
	}
	
	return img;
}

char * decode (struct PPM *orig, struct PPM *encoded) {
	if (orig->h != encoded->h || orig->w != encoded->w) return 0;
	int current = 0;
	int size = 20;
	char *out = (char *) malloc(size);
	char temp = 0;
	
	int i;
	int c = 0;
	
	while(c<3*orig->h*orig->w) {
		//printf("checking pixel %d -> %d = %d\n", c/3, orig->imageMatrix[c], encoded->imageMatrix[c]);
		if((orig->imageMatrix[c] & 1) ^ (encoded->imageMatrix[c] & 1)) {
			//printf("found altered pixel %d\n", c);
			if(current >= size-1) {
				size += 20;
				out = realloc(out, size);
			}
			for(i=0;i<8;i++) {
				temp = (temp << 1) | (encoded->imageMatrix[c+8-i] & 1);
			}
			c+=9;
			out[current] = temp;
			current++;
		} else c+=3;
	}
	for(i=current;i<size-1;i++) out[i] = (char) 0;
	out[size-1] = '\0';
	return out;
}

int main (int argc, char *argv[]) {
    FILE *file1;
    
    if(!(file1=fopen(argv[2],"r"))){
        printf("Error opening file %s\n", argv[2]);
        exit(1);
    }
    
    struct PPM *file1PPM;
    if(!(file1PPM = getPPM(file1))){
		printf("Error reading file\n");
		exit(1);
	}
	
    if(*argv[1] == 'e'){
        if (argc != 3){
            printf("Wrong number of arguments\n");
            exit(1);
        }
        char *str;
        char *temp;
        temp = (char*) malloc(50);
        str = (char*) malloc(sizeof(char));
        fprintf(stderr, "Enter string to de encoded:\n");
        while(scanf("%50[^\n]", temp)){
			str = (char*) realloc(str, strlen(str)+50*sizeof(char));
			strncpy(str, strcat(str, temp), sizeof(str)+sizeof(temp));
		}
		struct PPM *encoded = encode(str, file1PPM);
		if (encoded != NULL) showPPM(encoded);
		else printf("Could not encode '%s' into %s\n", str, argv[2]);
    }
	
    if(*argv[1] == 'd'){
		if (argc != 4){
			printf("Wrong number of arguments");
			exit(1);
		}
		FILE *file2;
		if (!(file2 = fopen(argv[3],"r"))){
			printf("Error opening output file\n");
			exit(1);
		}
		struct PPM *file2PPM = getPPM(file2);
		char *result = decode(file1PPM, file2PPM);
		printf("%s\n", result);
	}
	
	return 0;
}
