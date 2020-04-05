#include <stdio.h>
#include <string.h>
#include <vector>

char levels[255];
int commentlevel = 0;
std::vector<char*> Lines;

#define STR_LEN 4096


int main(int argc, char** argv)
{

	if (argc<2) 
	{
		fprintf(stderr,"Please enter the filename!");
		return 2;
	}

	FILE *f = fopen(argv[1],"rt");
	if (!f) return 1;
	

	levels[0] = false;

//	char* line = new char[STR_LEN + 1];

	char* word1 = new char[255];
	char* word2 = new char[255];

	char* tline = new char[STR_LEN + 1];
	char* tline2 = new char[STR_LEN + 1];

		int level = 0;
		levels[0] = 2;

	bool dwt = false;
	int lnum = 0;

	while (!feof(f))
	{	
		char* line = new char[STR_LEN + 1];
		if (!fgets(line,STR_LEN,f)) break;

		Lines.push_back(line);
		lnum++;
	}

	fclose(f);
	f = fopen(argv[1],"wt");
	if (!f) return 1;

	lnum = 0;


	for (int i=0;i<Lines.size();i++)
	{	
		dwt = false;

//		if (!fgets(line,STR_LEN,f)) break;
		char* line = Lines[i];
		lnum++;

		if (level<0) 
			fprintf(stderr,"PANIC! LEVEL < 0 \n");
			

		line[STR_LEN]='\0';
		strcpy(tline,line);

		if (line[0]==' ' || line[0]=='\t') strcpy(line,line+1);

		bool dropline = false;
		int slen = strlen(line);

		for(int i=0;i<slen;i++)
		{
			if (i<slen-1)
			{
				if (line[i]=='/' && line[i+1]=='*') 
				{
					commentlevel++;
					i++;
				}
				else
				if (line[i]=='*' && line[i+1]=='/')
				{
					commentlevel--;
					i++;
				}
				else
				if (line[i]=='/' && line[i+1]=='/') dropline = true;

				if (dropline)
				{
					line[i]=' ';
					line[i+1]=' ';
				}
			}
			
			if (commentlevel>0) line[i]=' ';
		}

		while (line[0]==' ' || line[0]=='\t') strcpy(line,line+1);


		if (commentlevel<0) 
			fprintf(stderr,"PANIC! COMMENTLEVEL < 0 \n");


		int l = sscanf(line,"%s %s",word1,word2);
		if ( l == 1)
		{	
			sscanf(line,"%s",word1);
			strcpy(word2,"");
		}
		else
		if (l < 1)
		{
			strcpy(word1,"");
			strcpy(word2,"");
		}
		

			if (!strcmp(word1,"#if"))
			{
				if (!strcmp(word2,"1"))
				{
					level++;
					if (levels[level-1]==2) levels[level] = 1;
					else levels[level] = levels[level-1];

					dwt = true;
				}
				else
				if (!strcmp(word2,"0"))
				{
					level++;
					levels[level] = 0;
				}
				else
				{
					int oldlev = levels[level];
					level++;
					if (oldlev == 0) levels[level] = 3;
					else if (oldlev == 3) levels[level] = 3;
					else levels[level] = 2;
				}
			}
			else
			if (!strcmp(word1,"#else") && level>0) 
			{
				switch(levels[level])			
				{
					case 0: 
						{
							levels[level] = 1;
							dwt = true;
							break;
						}
					case 1: levels[level] = 0;break;
				}
			}
			else
			if (!strcmp(word1,"#endif")) 
			{
				dwt = (levels[level] < 2);
				levels[level] = 0;
				level--;
			}
			else
			if ((!strcmp(word1,"#ifdef")) || (!strcmp(word1,"#ifndef")) ) 
			{
				int oldlev = levels[level];
				level++;
				if (oldlev==0 || oldlev==3) levels[level] = 3;
				else levels[level] = 2;
			};


		if ((levels[level]) && !(levels[level] == 3) && !dwt) fprintf(f,"%s",tline);
//		printf("[%i,%i,%i] %s",lnum,level,commentlevel,tline);


	}
	
	fclose(f);


	return 0;
}