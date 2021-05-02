 #define DEBUG_TIME_BLOCK()
#include "easy_types.h"
#include "easy_array.h"
#include "easy_unicode.h"
#include "easy_platform.h"
#include "easy_string.h"
#include "easy_files.h"

typedef struct {
	InfiniteAlloc contentsToWrite;
} FileState;

static initFileState(FileState *state) {
	state->contentsToWrite = initInfinteAlloc(u8);
}

static void outputFile(FileState *state, char *filename) {
	FILE *file = fopen(filename, "w+");

	size_t sizeWritten = fwrite(state->contentsToWrite.memory, 1, state->contentsToWrite.count, file);

	fclose(file);
}

static char *folderName1 = "C:\\Users\\olive\\Downloads\\185-Yoga-Poses\\Y2020 Black Hair\\Y2020_black_hair png\\";

int main(int argc, char *args[]) {

	// if(argc > 1) 
	{
		char *folderName = folderName1;//args[1];

		char *exts[] = { "png" };
		FileNameOfType files = getDirectoryFilesOfType(folderName, exts, 1);

		printf("folder name: %s\n", folderName);
		printf("file count: %d\n", files.count);

		char *typeStr = "Yoga_Pose_";

		FileState state;
		initFileState(&state);

		for(int i = 0; i < files.count; ++i) {
			char *name = files.names[i];

			char *shortName = getShortName(name);

			/////////// replace underscores /////////
			char *typeString = easyString_copyToHeap(shortName);

			
			char *at = typeString;
			while(*at) {
				if(*at == ' ') {
					*at = '_';
				}
				at++;
			}
			///////////////////////////////

			int lengthOfStr = easyString_getSizeInBytes_utf8(typeStr) + easyString_getSizeInBytes_utf8(typeString) + 3;
			char *s = (char *)malloc(lengthOfStr);
			
				
			int size = easyString_getSizeInBytes_utf8(typeStr);
			for(int k = 0; k < size; ++k) {
				s[k] = typeStr[k];
			}

			for(int k = 0; k < easyString_getSizeInBytes_utf8(typeString); ++k) {
				s[k + size] = typeString[k];
			}

			s[lengthOfStr - 3] = ',';
			s[lengthOfStr - 2] = '\n';
			s[lengthOfStr - 1] = '\0';

			addElementInifinteAllocWithCount_(&state.contentsToWrite, s, lengthOfStr -1 );
			
			///////////////////////////////
			
		} 
		for(int i = 0; i < files.count; ++i) {
			char *initString = easy_createString_printf_needToFree("yogaSprites[%d] = (Sprite)(a.objs[%d]);\n", i, i*2 + 1);
			addElementInifinteAllocWithCount_(&state.contentsToWrite, initString, easyString_getSizeInBytes_utf8(initString));
		}

		char *arrayString = easy_createString_printf_needToFree("poses = new YogaPose[%d];\n", files.count);
		addElementInifinteAllocWithCount_(&state.contentsToWrite, arrayString, easyString_getSizeInBytes_utf8(arrayString));
		for(int i = 0; i < files.count; ++i) {
			char *name = files.names[i];

			char *shortName_withFileType = getShortName_withFileExtension(name);
			// printf("%s\n", shortName_withFileType);
			char *shortName = getShortName(name);
			// printf("%s\n", shortName);

			/////////// replace underscores /////////
			char *typeString = easyString_copyToHeap(shortName);

			
			char *at = typeString;
			while(*at) {
				if(*at == ' ') {
					*at = '_';
				}
				at++;
			}
			///////////////////////////////

			int lengthOfStr = easyString_getSizeInBytes_utf8(typeStr) + easyString_getSizeInBytes_utf8(typeString) + 3;
			char *s = (char *)malloc(lengthOfStr);
			
				
			int size = easyString_getSizeInBytes_utf8(typeStr);
			for(int k = 0; k < size; ++k) {
				s[k] = typeStr[k];
			}

			for(int k = 0; k < easyString_getSizeInBytes_utf8(typeString); ++k) {
				s[k + size] = typeString[k];
			}

			s[lengthOfStr - 3] = ',';
			s[lengthOfStr - 2] = '\n';
			s[lengthOfStr - 1] = '\0';

			char *typeOfString_withoutNewLine = easyString_copyToHeap(s);

			typeOfString_withoutNewLine[lengthOfStr - 3] = '\0';

			// addElementInifinteAllocWithCount_(&state.contentsToWrite, s, lengthOfStr);
			// printf("%s", s);

			//////////////////////////////// Output the yoga poses constructors //////////////////////////////////////////

			// char *spritePath = easy_createString_printf_needToFree("Resources.Load<Sprite>(\"%s\")", shortName);
			// char *audioPath = easy_createString_printf_needToFree("Resources.Load<AudioClip>(\"%s\")", "cat");

			char *constructor = easy_createString_printf_needToFree("poses[%d] = new YogaPose(\"\", \"%s\", gameController.yogaSprites[%d], %s, YogaPoseType.%s, false);\n", i, shortName, i, "null", typeOfString_withoutNewLine);
			// addElementInifinteAllocWithCount_(&state.contentsToWrite, s, lengthOfStr);

			addElementInifinteAllocWithCount_(&state.contentsToWrite, constructor, easyString_getSizeInBytes_utf8(constructor));

			///////////////////////////////
			
		} 

		// printf("%s\n", (char *)state.contentsToWrite.memory);
		outputFile(&state, ".\\output.txt");
	}
	 // else {
	// 	printf("%s\n", "no foldner name passed");
	// }

}