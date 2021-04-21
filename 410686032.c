#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include<ctype.h>
#include<time.h>
#define SWAP(x,y) {int t; t = x; x = y; y = t;}

#define PI 3.1415926


typedef struct Bmpheader {
	char identifier[2]; // 0x0000
	unsigned int filesize; // 0x0002
	unsigned short reserved; // 0x0006
	unsigned short reserved2;
	unsigned int bitmap_dataoffset; // 0x000A
	unsigned int bitmap_headersize; // 0x000E
	unsigned int width; // 0x0012
	unsigned int height; // 0x0016
	unsigned short planes; // 0x001A
	unsigned short bits_perpixel; // 0x001C
	unsigned int compression; // 0x001E
	unsigned int bitmap_datasize; // 0x0022
	unsigned int hresolution; // 0x0026
	unsigned int vresolution; // 0x002A
	unsigned int usedcolors; // 0x002E
	unsigned int importantcolors; // 0x0032
	unsigned int palette; // 0x0036
} Bitmap;


void readheader(FILE* fp, Bitmap *x) {
	fread(&x->identifier, sizeof(x->identifier), 1, fp);
	fread(&x->filesize, sizeof(x->filesize), 1, fp);
	fread(&x->reserved, sizeof(x->reserved), 1, fp);
	fread(&x->reserved2, sizeof(x->reserved2), 1, fp);
	fread(&x->bitmap_dataoffset, sizeof(x->bitmap_dataoffset), 1, fp);
	fread(&x->bitmap_headersize, sizeof(x->bitmap_headersize), 1, fp);
	fread(&x->width, sizeof(x->width), 1, fp);
	fread(&x->height, sizeof(x->height), 1, fp);
	fread(&x->planes, sizeof(x->planes), 1, fp);
	fread(&x->bits_perpixel, sizeof(x->bits_perpixel), 1, fp);
	fread(&x->compression, sizeof(x->compression), 1, fp);
	fread(&x->bitmap_datasize, sizeof(x->bitmap_datasize), 1, fp);
	fread(&x->hresolution, sizeof(x->hresolution), 1, fp);
	fread(&x->vresolution, sizeof(x->vresolution), 1, fp);
	fread(&x->usedcolors, sizeof(x->usedcolors), 1, fp);
	fread(&x->importantcolors, sizeof(x->importantcolors), 1, fp);

}

typedef struct RGB{
	unsigned char R;
	unsigned char G;
	unsigned char B;
} ImgRGB;

typedef struct YUV{
	short Y;
	short U;
	short V;
} ImgYUV;

typedef struct table{
	short nonzero;
	short value;
	short total_size;
}AC_table;


typedef struct huff{
	int zeros;
	int size;
}pre_huff;

typedef struct node{
	short val;
	int count;
	short explored;
	struct node* left, *right;
}treeNodestruct, *PtreeNodestruct;

typedef struct code{
	char bits[16];
	short val;
}codes;


ImgYUV *allocating_memory(unsigned int Total_Sample){
	return (ImgYUV*)malloc(sizeof(ImgYUV) * Total_Sample);
}
char *allocating_memory_char(unsigned int Total_Sample){
	return (char*)malloc(sizeof(char) * Total_Sample);
}

AC_table *allocating_memory_AC(unsigned int Total_Sample){
	return (AC_table*)malloc(sizeof(AC_table) * Total_Sample);
}

pre_huff *allocating_memory_huff(unsigned int Total_Sample){
	return (pre_huff*)malloc(sizeof(pre_huff) * Total_Sample);
}


short *allocating_memory_short(unsigned int Total_Sample){
	return (short*)malloc(sizeof(short) * Total_Sample);
}

int *allocating_memory_int(unsigned int Total_Sample){
	return (int*)malloc(sizeof(int) * Total_Sample);
}



codes *allocating_memory_codes(unsigned int Total_Sample){
	return (codes*)malloc(sizeof(codes) * Total_Sample);
}

treeNodestruct *allocating_memory_treeNodestruct(unsigned int Total_Sample){
	return (treeNodestruct*)malloc(sizeof(treeNodestruct) * Total_Sample);
}



void sort(int *count, pre_huff * table, short bound){
	for (int i = 0; i < bound; i++){
		if (table[i].zeros != 16){
			count[table[i].zeros + table[i].size]++;
		}
		else if (table[i].zeros == 16){
			count[0]++;
		}
	}
}

short assign(int *count, treeNodestruct *freq){
	short k = 0;
	for (int i = 0; i < 151; i++){
		if (count[i] >0){
			freq[k].val = i;
			freq[k].count = count[i];
			freq[k].left = NULL;
			freq[k].right = NULL;
			freq[k].explored = 0;
			k++;
		}
	}
	return k;
}


int partition(treeNodestruct *number, int left, int right) {
    int i = left - 1;
    int j;
    for(j = left; j < right; j++) {
        if(number[j].count <= number[right].count) {
            i++;
            SWAP(number[i].count, number[j].count);
            SWAP(number[i].val, number[j].val)
        }
    }

    SWAP(number[i+1].count, number[right].count);
    SWAP(number[i+1].val, number[right].val)
    return i+1;
}

void quickSort(treeNodestruct *number, int left, int right) {
    if(left < right) {
        int q = partition(number, left, right);
        quickSort(number, left, q-1);
        quickSort(number, q+1, right);
    }
}

void huff_tree(treeNodestruct *number, short count, treeNodestruct *parents){
	short index = 0, index_num = 2;
	parents[index].left = &number[0];
	parents[index].right = &number[1];
	parents[index].count = number[0].count +number[1].count;

	while (index < count){
		if (parents[index].count >= number[index_num].count){
			index++;
			parents[index].left = &parents[index-1];
			parents[index].right = &number[index_num];
			parents[index].count = parents[index-1].count + number[index_num].count;
			index_num++;
		}
		else if (parents[index].count < number[index_num].count){
			index++;
			parents[index].right = &parents[index-1];
			parents[index].left = &number[index_num];
			parents[index].count = parents[index-1].count + number[index_num].count;
			index_num++;
		}
	}

}



void InputData(FILE* fp, ImgRGB **array, int H, int W, int skip){
	int temp;
	char skip_buf[3];
	int i, j;
	for (i = 0; i<H; i++){
		for (j = 0; j<W; j++){
			temp = fgetc(fp);
			array[i][j].B = temp;
			temp = fgetc(fp);
			array[i][j].G = temp;
			temp = fgetc(fp);
			array[i][j].R = temp;
		}
		if (skip != 0) fread(skip_buf, skip, 1, fp);
	}
	printf("\n"); // DEBUG
}

short** malloc_2D_short(int row, int col){
	short **Array, *Data;
	int i;
	Array = (short**)malloc(row*sizeof(short *));
	Data = (short*)malloc(row*col*sizeof(short));
	for (i = 0; i<row; i++, Data += col){
		Array[i] = Data;
	}
	return Array;
}


ImgRGB** malloc_2D(int row, int col){
	ImgRGB **Array, *Data;
	int i;
	Array = (ImgRGB**)malloc(row*sizeof(ImgRGB *));
	Data = (ImgRGB*)malloc(row*col*sizeof(ImgRGB));
	for (i = 0; i<row; i++, Data += col){
		Array[i] = Data;
	}
	return Array;
}


ImgYUV** malloc_2D_1(int row, int col){
	ImgYUV **Array, *Data;
	int i;
	Array = (ImgYUV**)malloc(row*sizeof(ImgYUV *));
	Data = (ImgYUV*)malloc(row*col*sizeof(ImgYUV));
	for (i = 0; i<row; i++, Data += col){
		Array[i] = Data;
	}
	return Array;
}

ImgYUV*** malloc_3D(int row, int col, int height){
	ImgYUV ***dim1, **dim2, *dim3;
	dim1 = (ImgYUV***)malloc(row*sizeof(ImgYUV **));
	dim2 = (ImgYUV**)malloc(row*col*sizeof(ImgYUV *));
	dim3 = (ImgYUV*)malloc(row*col*height*sizeof(ImgYUV));
	for(int i = 0 ; i < row; i++, dim2 +=col){
		dim1[i] = dim2;
		for(int j = 0 ; j< col; j++, dim3 +=height){
			dim1[i][j] = dim3;
		}
	}
	return dim1;
}


// this function can transform RGB format to XYZ format and in this program i'll call it YUV  (color space transform)
// it's just basic calculation with certain variable
void YUV_cal(ImgYUV **YUV, ImgRGB **RGB, int H, int W){
	float temp;
	for (int i = 0 ; i < H; i++){
		for (int j = 0 ; j < W; j++){
			temp = (RGB[i][j].R * 0.299 + RGB[i][j].G * 0.587  + RGB[i][j].B * 0.114) ;
			YUV[i][j].Y =round (temp);
			temp = (RGB[i][j].R * (-0.168) + RGB[i][j].G * (-0.331)  + RGB[i][j].B * 0.499) ;
			YUV[i][j].U =round (temp);
			temp = RGB[i][j].R * 0.5 + RGB[i][j].G * (-0.419)  + RGB[i][j].B * (-0.081);
			YUV[i][j].V =round (temp);

		}
	}
}


// major function that calculates discrete cosine transform
// ß‚Æ…∞Ï¬‡®Ï¿W≤v∂b
// i use DCT 2 in this function
void DCT(ImgYUV **YUV, short N, ImgYUV ** YUV_out, short frame, short frame1){
	float temp, temp_1, temp_2;
	float buff[4096];
	for (int l1 = 0,index =0 ; l1 < 8; l1++){
		for (int l2 = 0 ; l2< 8; l2++){
			for (int l3 = 0 ; l3 < 8; l3++){
				for (int l4 = 0 ; l4< 8; l4++,index++){
					buff[index] =  cos(((2*l3 + 1) * l1 * PI)/(2*N)) * cos(((2*l4 + 1) * l2 * PI)/(2*N));
				}
			}
		}
	}
	short index = 0;
	for (int f = 0; f < frame; f++){
		for (int f1 = 0; f1 < frame1; f1++, index = 0){
			for (int i = 0 ; i < N; i++ ){
				for (int j = 0 ; j < N; j++){
					temp = 0;
					temp_1 = 0;
					temp_2 = 0;
					for (int x = 0; x < N ; x++){
						for(int y = 0 ; y < N; y++,index++){
							temp += (float)(YUV[x + (8 * f)][y + (8 * f1)].Y - 128) * buff[index];
							temp_1 += (float)(YUV[x + (8 * f)][y + (8 * f1)].U - 128) * buff[index];
							temp_2 += (float)(YUV[x + (8 * f)][y + (8 * f1)].V - 128) * buff[index];
						}
					}
					if ( i == 0 && j == 0){
						temp *= 0.125;
						temp_1 *= 0.125;
						temp_2 *= 0.125;
						YUV_out[i + (8 * f)][j + (8 *f1)].Y = (short)round(temp);
						YUV_out[i + (8 * f)][j + (8 *f1)].U = (short)round(temp_1);
						YUV_out[i + (8 * f)][j + (8 *f1)].V = (short)round(temp_2);
					}
					else if (i == 0 || j == 0){
						temp *= 0.1767;
						temp_1 *= 0.1767;
						temp_2 *= 0.1767;
						YUV_out[i + (8 * f)][j + (8 *f1)].Y = (short)round(temp);
						YUV_out[i + (8 * f)][j + (8 *f1)].U = (short)round(temp_1);
						YUV_out[i + (8 * f)][j + (8 *f1)].V = (short)round(temp_2);
					}
					else{
						temp *= 0.25 ;
						temp_1 *= 0.25 ;
						temp_2 *= 0.25 ;
						YUV_out[i + (8 * f)][j + (8 *f1)].Y = (short)round(temp);
						YUV_out[i + (8 * f)][j + (8 *f1)].U = (short)round(temp_1);
						YUV_out[i + (8 * f)][j + (8 *f1)].V = (short)round(temp_2);
					}
				}
			}
		}
	}
}



// i searched for an quantization table online and apply it to my function
// this function basically just removes high frequencies component
void quan(ImgYUV **YUV, short frame, short frame1){
	short qua_array[8][8] = {
			{16,11,10,16,24,40,51,61},
			{12,12,14,19,26,58,60,55},
			{14,13,16,24,40,57,69,56},
			{14,17,22,29,51,87,80,62},
			{18,22,37,56,68,109,103,77},
			{24,35,55,64,81,104,113,92},
			{49,64,78,87,103,121,120,101},
			{72,92,95,98,112,100,103,99}
	};
	for (int i = 0 ; i < frame; i ++){
		for (int j = 0 ; j <frame1; j++){
			for (int x = 0 ; x < 8; x ++){
				for (int y = 0 ; y < 8 ; y++){
					YUV[x + (8 * i)][y + (8 * j)].Y = round(YUV[x + (8 * i)][y + (8 * j)].Y / qua_array[x][y]);
					YUV[x + (8 * i)][y + (8 * j)].U = round(YUV[x + (8 * i)][y + (8 * j)].U / qua_array[x][y]);
					YUV[x + (8 * i)][y + (8 * j)].V = round(YUV[x + (8 * i)][y + (8 * j)].V / qua_array[x][y]);
				}
			}
		}
	}
}

void zigzag(ImgYUV ** YUV, ImgYUV ***ziazag_YUV, short frame, short frame1){
	short x, y, i , j;
	short table[8][8] = {
			{0,1,5,6,14,15,27,28},
			{2,4,7,13,16,26,29,42},
			{3,8,12,17,25,30,41,43},
			{9,11,18,24,31,40,44,53},
			{10,19,23,32,39,45,52,54},
			{20,22,33,38,46,51,55,60},
			{21,34,37,47,50,56,59,61},
			{35,36,48,49,57,58,62,63}
	};
	for (x = 0 ; x < frame; x++){
		for(y = 0 ; y < frame1; y++){
			for(i = 0 ; i < 8; i ++){
				for(j = 0; j< 8; j++){
					ziazag_YUV[x][y][table[i][j]].Y = YUV[i + (8 * x)][j + (8 * y)].Y;
					ziazag_YUV[x][y][table[i][j]].U = YUV[i + (8 * x)][j + (8 * y)].U;
					ziazag_YUV[x][y][table[i][j]].V = YUV[i + (8 * x)][j + (8 * y)].V;
				}
			}
		}
	}
}

void dif(ImgYUV *diff, short frame, short frame1, ImgYUV ***zigzag_YUV){
	int i, j, x;
	x = 0;
	short temp, temp1, temp2;
	for (i = 0; i < frame; i++){
		for(j = 0 ; j < frame1; j++, x++){
			if ( x == 0){
				diff[x].Y = zigzag_YUV[i][j][0].Y;
				diff[x].U = zigzag_YUV[i][j][0].U;
				diff[x].V = zigzag_YUV[i][j][0].V;
				temp = zigzag_YUV[i][j][0].Y;
				temp1 = zigzag_YUV[i][j][0].U;
				temp2 = zigzag_YUV[i][j][0].V;
			}
			else{
				diff[x].Y = zigzag_YUV[i][j][0].Y - temp;
				diff[x].U = zigzag_YUV[i][j][0].U - temp1;
				diff[x].V = zigzag_YUV[i][j][0].V - temp2;
				temp = zigzag_YUV[i][j][0].Y;
				temp1 = zigzag_YUV[i][j][0].U;
				temp2 = zigzag_YUV[i][j][0].V;
			}
		}
	}
}


//transform 63 AC number into the form of compressed information
void ACC (short * AC, AC_table * table){
	short count = 0, i = 0, ta = 0;
	table[1].total_size = 0;
	for (i = 0 ; i < 63; i++){
		if (AC[i] == 0){
			count++;
			if(count > 15){
				table[1].total_size = 1;
				break;
			}
		}
		else{
			table[ta].value = AC[i];
			table[ta].nonzero = count;
			count = 0;
			ta += 1;
		}
	}

	table[ta].nonzero = count;
	table[0].total_size = ta;
}



void huffman_generate(short * AC , pre_huff * table){
	int count = 0, i = 0, ta = 0;
	for (i = 0 ; i < 63 ; i++){
		if (AC[i] == 0 ){
			count++;
			if (count >15){
				table[ta].zeros = 16;
				break;
			}
		}
		else{
			table[ta].size = floor(log2(abs(AC[i]))) + 1;
			table[ta].zeros = count;
			count = 0;
			ta++;
		}
	}
}

// takes in one value and transform it into bitstream form and write it in str
void DCCODE(char *str, short dc){
	char result[20];
	short index, i;
	char arr[12][9] = {
	"00","010","011","100","101","110","1110","11110","111110","1111110","11111110","111111110"
	};
	char bi[11];
	if (dc==0){
		strncpy(result, "00", 2);
		strcat(str,result);
		memset(result,0,sizeof(result));
	}
	else if (dc > 0){
		index = floor(log2(abs(dc)));
		itoa(dc,bi,2);
		strcpy(result, arr[index+1]);
		strcat(result,bi);
		strcat(str, result);
		memset(bi, 0 , sizeof(bi));
		memset(result,0,sizeof(result));
	}
	else{
		index = floor(log2(abs(dc)));
		itoa(abs(dc),bi,2);
		for(i = 0; bi[i] !='\0'; i++){
			if(bi[i] == '0')
				bi[i] = '1';
			else if (bi[i] == '1'){
				bi[i] = '0';
			}
		}
		strcpy(result, arr[index+1]);
		strcat(result,bi);
		strcat(str, result);
		memset(bi, 0 , sizeof(bi));
		memset(result,0,sizeof(result));
	}
}



//str is where to store all the information   AC is the compresssed information from 63 bits of AC value
void ACCODE(char *str, AC_table * AC){
	short index = 0, i = 0, set = 0;
	char result[30];
	char bi[15];
	memset(result, 0, sizeof(result));
	memset(bi,0,sizeof(bi));
	char arr[16][10][17] = {
			{"00","01","100","1011","11010","1111000","11111000","1111110110","1111111110000010","1111111110000011"},
			{"1100","11011","1111001","111110110", "11111110110","1111111110000100","1111111110000101","1111111110000110","1111111110000111","1111111110001000"},
			{"11100","11111001","1111110111","111111110100","1111111110001001","1111111110001010","1111111110001011","1111111110001100","1111111110001101","1111111110001110"},
			{"111010","111110111","111111110101","1111111110001111","1111111110010000","1111111110010001","1111111110010010","1111111110010011","1111111110010100","1111111110010101"},
			{"111011","1111111000","1111111110010110","1111111110010111","1111111110011000","1111111110011001","1111111110011010","1111111110011011","1111111110011100","1111111110011101"},
			{"1111010","11111110111","1111111110011110","1111111110011111","1111111110100000","1111111110100001","1111111110100010","1111111110100011","1111111110100100","1111111110100101"},
			{"1111011","111111110110","1111111110100110","1111111110100111","1111111110101000","1111111110101001","1111111110101010","1111111110101011","1111111110101100","1111111110101101"},
			{"11111010","111111110111","1111111110101110","1111111110101111","1111111110110000","1111111110110001","1111111110110010","1111111110110011","1111111110110100","1111111110110101"},
			{"111111000","111111111000000","1111111110110110","1111111110110111","1111111110111000","1111111110111001","1111111110111010","1111111110111011","1111111110111100","1111111110111101"},
			{"111111001","1111111110111110","1111111110111111","1111111111000000","1111111111000001","1111111111000010","1111111111000011","1111111111000100","1111111111000101","1111111111000110"},
			{"111111010","1111111111000111","1111111111001000","1111111111001001","1111111111001010","1111111111001011","1111111111001100","1111111111001101","1111111111001110","1111111111001111"},
			{"1111111001","1111111111010000","1111111111010001","1111111111010010","1111111111010011","1111111111010100","1111111111010101","1111111111010110","1111111111010111","1111111111011000"},
			{"1111111010","1111111111011001","1111111111011010","1111111111011011","1111111111011100","1111111111011101","1111111111011110","1111111111011111","1111111111100000","1111111111100001"},
			{"11111111000","1111111111100010","1111111111100011","1111111111100100","1111111111100101","1111111111100110","1111111111100111","1111111111101000","1111111111101001","1111111111101010"},
			{"1111111111101011","1111111111101100","1111111111101101","1111111111101110","1111111111101111","1111111111110000","1111111111110001","1111111111110010","1111111111110011","1111111111110100"},
			{"1111111111110101","1111111111110110","1111111111110111","1111111111111000","1111111111111001","1111111111111010","1111111111111011","1111111111111100","1111111111111101","1111111111111110"}
	};
short tem =0;
short bond;
if (AC[1].total_size == 1){
	bond = AC[0].total_size;
}
if (AC[1].total_size == 0){
	bond = AC[0].total_size -1;
}
	while (i <= bond && set != 1){
		tem = AC[i].value;
		if (tem > 0 &&AC[i].nonzero < 15){
			strcpy(result, bi);
			index = floor(log2(tem));
			itoa(tem,bi,2);
			strcat(result, arr[AC[i].nonzero][index]);
			strcat(result, bi);
			strcat(str, result);
			memset(bi, 0 , sizeof(bi));
			memset(result,0,sizeof(result));
		}
		else if (tem < 0&&AC[i].nonzero < 15){
			index = floor(log2(abs(tem)));
			itoa(abs(tem),bi,2);
			for(int j = 0; bi[j] !='\0'; j++){
				if(bi[j] == '0')
					bi[j] = '1';
				else if (bi[j] == '1')
					bi[j] = '0';
				}
			strcat(result, arr[AC[i].nonzero][index]);
			strcat(result, bi);
			strcat(str, result);
			memset(bi, 0, sizeof(bi));
			memset(result,0,sizeof(result));
		}
		else if (AC[i].nonzero ==15){
				strcat(result,"11111111001");
				strcat(str,result);
				memset(result,0,sizeof(result));
			}
		else if (AC[i].nonzero >15){
			strcat(result,"1010");
			strcat(str,result);
			memset(result,0,sizeof(result));
			set = 1;
		}
		i++;
	}
	if (set == 0){
		strcat(result,"1010");
		strcat(str,result);
		memset(result,0,sizeof(result));
	}
}




void digit_write(char *str, FILE *txt, int * set, FILE *header){
	set[3] = 0;
	unsigned char decimal_val = 0, base = 1, rem;
	int str_size = strlen(str),dec;
	short det = 0, offset = 0;
	long int i;
	char buffer [20];
	unsigned short bu;
	int head = 0;

	fseek(txt, SEEK_SET,0);
	fwrite(&head,sizeof(head),1,txt);
	fseek(txt, SEEK_SET,4);
	fwrite(&head,sizeof(head),1,txt);
	fseek(txt, SEEK_SET,8);
	fwrite(&head,sizeof(head),1,txt);
	fseek(txt, SEEK_SET,12);
	fwrite(&head,sizeof(head),1,txt);
	fseek(txt, SEEK_SET,16);

	for (i = 0 ; i < str_size; i+=8){
		memcpy(buffer, &str[i], 8);
		dec = atoi(buffer);
		    while (dec > 0)
		    {
		        rem = dec % 10;
		        decimal_val = decimal_val + rem * base;
		        dec = dec / 10 ;
		        base = base * 2;
		    }
		    if (det == 0){
		    	bu = decimal_val;
		    	det = 1;
		    	goto ski;
		    }
		    if (det == 1){
		    	bu = bu * 256 + decimal_val;
		    	fwrite(&bu,sizeof(bu),1, txt);
		    	bu = 0;
		    	det = 0;

		    }
		    ski:
			if (i >= str_size-8){
				if (atoi(buffer) != 0 ){
					offset = (strlen(buffer)-1) - floor(log10(atoi(buffer)));
					fwrite(&bu,sizeof(bu),1,txt);
				}
				if(atoi(buffer) == 0){
					offset = (strlen(buffer))- 1;
					fwrite(&bu,sizeof(bu),1,txt);
					set[3] = 1;
				}

			}
		    base = 1, decimal_val = 0;
	}
	set[0] = offset;
	if ((int)ceil(str_size/8)% 2 == 0)set[1] =(int)ceil(str_size/8) +  2;
	else {set[1] =(int)ceil(str_size/8) +  1;}//16 bit reform
	set[2] = str_size;

	fseek(txt, SEEK_SET,0);
	head = set[0];
	fwrite(&head,sizeof(head),1,txt);
	head = set[1];
	fseek(txt, SEEK_SET,4);
	fwrite(&head,sizeof(head),1,txt);
	head = set[2];
	fseek(txt, SEEK_SET,8);
	fwrite(&head,sizeof(head),1,txt);
	head = set[3];
	fseek(txt, SEEK_SET,12);
	fwrite(&head,sizeof(head),1,txt);
}

// read from txt file and save in string str, set[0] = offset , set[1] = total byte being written


void digit_read(FILE *txt, char *str, FILE *header){
	unsigned short bu1, rem,count = 0,se = 0;
	int pt = 0;
	char * buffer = NULL;
	buffer = allocating_memory_char(16);
	char * buffer1 = NULL;
	buffer1 = allocating_memory_char(16);
	char * buffer2 = NULL;
	buffer2 = allocating_memory_char(16);
	memset(buffer,'\0',16);
	memset(buffer1,'\0',16);
	memset(buffer2,'\0',16);
	int set0, set1, set2, set3;

	fseek(txt, SEEK_SET,0);
	fread(&set0, sizeof(set0), 1, txt);

	fseek(txt, SEEK_SET,4);
	fread(&set1, sizeof(set1), 1, txt);

	fseek(txt, SEEK_SET,8);
	fread(&set2, sizeof(set2), 1, txt);

	fseek(txt, SEEK_SET,12);
	fread(&set3, sizeof(set3), 1, txt);


	int i = 0;
	fseek(txt, SEEK_SET,0);
	for (i = 0; i < set1; i += 2){
		fseek(txt, i+16 ,SEEK_CUR);
		fread(&bu1, sizeof(bu1), 1, txt);
		fseek(txt, SEEK_SET,0);
		for (; bu1 !=0 ; bu1 = bu1/2){
			rem = bu1%2;
			count +=1;
			if (rem == 1)strcat(buffer,"1");
			else if(rem == 0)strcat(buffer,"0");
		}
		for (;count <16; count++){
			strcat(buffer,"0");
		}
		buffer = strrev(buffer);
		strcat(str,buffer);
		memset(buffer,'\0',16);
		count = 0;
	}

	if ((int)ceil(set2/8)% 2 != 0 && set3 == 0){
		memcpy(buffer, &str[strlen(str)-8],8);
		for (int de = 0 ; de < 8; de ++){
			if (buffer[de] == '1' && se == 0)pt = de, se = 1;
		}
		for (int quan = 0 ; quan < set0; quan ++){
			strcat(buffer2,"0");
		}
		memcpy(buffer1, &buffer[pt],8-pt);
		strcat(buffer2, buffer1);
		memcpy(&str[strlen(str)-8],buffer2,8);
	}
	else if ((int)ceil(set2/8)% 2 == 0 && set3 == 0){
		short co = 0;
		memcpy(buffer, &str[strlen(str)-16],16);

		for (int de = 0 ; de < 16; de ++){
			if (buffer[de] == '1' && se == 0)pt = de, se = 1;
			else if (buffer[de] == '0')co++;
			printf("\n%c", buffer[de]);
		}
		for (int quan = 0 ; quan < set0; quan ++){
			strcat(buffer2,"0");
		}
		if (co == 16)pt = 16;
		memcpy(buffer1, &buffer[pt],16-pt);
		strcat(buffer2, buffer1);
		memcpy(&str[strlen(str)-16],buffer2,16);
	}
	else if ((int)ceil(set2/8)% 2 != 0 && set3 == 1){
		memcpy(buffer, &str[strlen(str)-8],8);
		memcpy(buffer1,&buffer[0],set0+1);
		memcpy(&str[strlen(str)-8],buffer1, 8);
	}
	else if ((int)ceil(set2/8)% 2 == 0 && set3 == 1){
		memcpy(buffer, &str[strlen(str)-16],16);
		memcpy(buffer1,&buffer[0],set0+1);
		memcpy(&str[strlen(str)-16],buffer1, 16);
	}
	if (strlen(str) > set2){
		memset(&str[set2], '\0', 1);
	}

}

short dec_bi(long int dec){
	unsigned char decimal_val = 0, base = 1, rem;
    while (dec > 0)
    {
        rem = dec % 10;
        decimal_val = decimal_val + rem * base;
        dec = dec / 10 ;
        base = base * 2;
    }
    return decimal_val;
}

short bi_dec(char * str){
	short val =0;
	for (int i = 0, j = strlen(str) - 1 ; i < strlen(str); i++, j--){
		if (str[i] == '1'){
		val += 1 * (short)pow(2, j);
		}
	}
	return val;
}

short bi_dec_ac(char * str){
	short val = 0;
	short offset = pow (2, strlen(str)) - 1;
	if (str[0] == '1'){
		for (int i = 0, j = strlen(str) - 1; i < strlen(str); i++, j--){
			if (str[i] == '1'){
				val += 1 *(short)pow(2,j);
			}
		}
	}
	else if (str[0] == '0'){
		for (int i = 0, j = strlen(str) - 1; i < strlen(str); i++, j--){
			if (str[i] == '1'){
				val += 1 *(short)pow(2,j);
			}
		}
		val = val - offset;
	}
	return val;
}



short decode(char * st, int *pt){
	short set = 0,x = 0,sam = 1;
	int i = pt[0], num = 0;
	short result[8][8];
	memset(result,'0',sizeof(result));
	char dc_arr[12][9] = {
	"00","010","011","100","101","110","1110","11110","111110","1111110","11111110","111111110"
	},buffer[20];
	memset(buffer,'\0',sizeof(buffer));
	while (set == 0){
		strncat(buffer, &st[i],1);
		while (sam != 0 && x < 12)sam = memcmp(buffer, &dc_arr[x],strlen(dc_arr[x])), x++;
		if (sam == 0){
			if (st[i+1] == '1'){
				memset(buffer,'\0',sizeof(buffer));
				memcpy(buffer, &st[i+1], x-1);
				pt[0] = i+x;
				num = bi_dec(buffer);
				return num;
			}
			else if (st[i+1] == '0'){
				memset(buffer, '\0', sizeof(buffer));
				memcpy(buffer, &st[i+1], x-1);
				for(int j = 0; j < x-1; j++){
					if(buffer[j] == '0')
						buffer[j] = '1';
					else if (buffer[j] == '1')
						buffer[j] = '0';
					}
				num = bi_dec(buffer);
				pt[0] = i+x;
				return 0 - num;
			}
			set = 1;
		}
		i++, x = 0;
	}
}

void AC_decode(char * st, int *pt, short *AC_com){
short x = 0,y = 0,sam = 1, count = 0,va= 0, pointer = 0;
int  i = pt[0];
short accmulation = 0;
short result[63];
memset(result, 0, sizeof(result));
char arr[16][10][17] = {
			{"00","01","100","1011","11010","1111000","11111000","1111110110","1111111110000010","1111111110000011"},
			{"1100","11011","1111001","111110110", "11111110110","1111111110000100","1111111110000101","1111111110000110","1111111110000111","1111111110001000"},
			{"11100","11111001","1111110111","111111110100","1111111110001001","1111111110001010","1111111110001011","1111111110001100","1111111110001101","1111111110001110"},
			{"111010","111110111","111111110101","1111111110001111","1111111110010000","1111111110010001","1111111110010010","1111111110010011","1111111110010100","1111111110010101"},
			{"111011","1111111000","1111111110010110","1111111110010111","1111111110011000","1111111110011001","1111111110011010","1111111110011011","1111111110011100","1111111110011101"},
			{"1111010","11111110111","1111111110011110","1111111110011111","1111111110100000","1111111110100001","1111111110100010","1111111110100011","1111111110100100","1111111110100101"},
			{"1111011","111111110110","1111111110100110","1111111110100111","1111111110101000","1111111110101001","1111111110101010","1111111110101011","1111111110101100","1111111110101101"},
			{"11111010","111111110111","1111111110101110","1111111110101111","1111111110110000","1111111110110001","1111111110110010","1111111110110011","1111111110110100","1111111110110101"},
			{"111111000","111111111000000","1111111110110110","1111111110110111","1111111110111000","1111111110111001","1111111110111010","1111111110111011","1111111110111100","1111111110111101"},
			{"111111001","1111111110111110","1111111110111111","1111111111000000","1111111111000001","1111111111000010","1111111111000011","1111111111000100","1111111111000101","1111111111000110"},
			{"111111010","1111111111000111","1111111111001000","1111111111001001","1111111111001010","1111111111001011","1111111111001100","1111111111001101","1111111111001110","1111111111001111"},
			{"1111111001","1111111111010000","1111111111010001","1111111111010010","1111111111010011","1111111111010100","1111111111010101","1111111111010110","1111111111010111","1111111111011000"},
			{"1111111010","1111111111011001","1111111111011010","1111111111011011","1111111111011100","1111111111011101","1111111111011110","1111111111011111","1111111111100000","1111111111100001"},
			{"11111111000","1111111111100010","1111111111100011","1111111111100100","1111111111100101","1111111111100110","1111111111100111","1111111111101000","1111111111101001","1111111111101010"},
			{"1111111111101011","1111111111101100","1111111111101101","1111111111101110","1111111111101111","1111111111110000","1111111111110001","1111111111110010","1111111111110011","1111111111110100"},
			{"1111111111110101","1111111111110110","1111111111110111","1111111111111000","1111111111111001","1111111111111010","1111111111111011","1111111111111100","1111111111111101","1111111111111110"}
	}, buffer[20];
	memset(buffer,'\0',sizeof(buffer));
	for (count = 0; count < 63; count+=accmulation){
		strncat(buffer, &st[i], 4);
		if (memcmp(buffer,"1010",4) == 0){
			i+=4;
			goto last;
		}
		memset(buffer,'\0',sizeof(buffer));
		strncat(buffer, &st[i],11);
		if (memcmp(buffer,"11111111001", 11) == 0){
			i += 11;
			short temp = pointer + 15;
			for (; pointer < temp; pointer++){
				AC_com[pointer] = 0;
			}
		}
		memset(buffer,'\0',sizeof(buffer));
		strncat(buffer, &st[i], 4);
		if (memcmp(buffer,"1010",4) == 0){
			i+=4;
			goto last;
		}
		memset(buffer,'\0',sizeof(buffer));
		accmulation = 0;
		strncat(buffer, &st[i], 1), i++;

		while(sam != 0 && isdigit(buffer[0]) == 1){
			sam = memcmp(buffer,arr[x][y],strlen(arr[x][y]));
			y++;
			if (y==10)y = 0, x++;
			if (x==16){
				x = 0;
				strncat(buffer, &st[i], 1), i++;
				continue;
			}
		}
		if (isdigit(buffer[0]) != 1)break;
		memset(buffer,'\0',sizeof(buffer));
		memcpy(buffer, &st[i], y);
		va = bi_dec_ac(buffer);
		for (int zero = 0 ; zero < x; zero++){
			AC_com[pointer] = 0;
			pointer++;
		}
		AC_com[pointer] = va;
		pointer++;
		i = i + y;
		accmulation = 1 + x;
		memset(buffer,'\0',sizeof(buffer));
		sam = 1, x = 0, y = 0;
	}
		last:
	for (; pointer < 63; pointer++) AC_com[pointer] = 0;
	pt[0] = i;
}

void de(char *st ,int *pt, short *dc, short **AC_com){
	int i = 0;
	while (st[pt[0]] != '\0'){
		dc[i] = decode(st, pt);
		AC_decode(st,pt, AC_com[i]);
		i++;
	}
}



void organize(ImgYUV **YUV, short *dc, short **AC_com, short height, short width, int *bound){
	short H = height * 8, W = width * 8, i = 0 , j = 0;
	int dc_index = 0;
	short table[8][8] = {
			{0,1,5,6,14,15,27,28},
			{2,4,7,13,16,26,29,42},
			{3,8,12,17,25,30,41,43},
			{9,11,18,24,31,40,44,53},
			{10,19,23,32,39,45,52,54},
			{20,22,33,38,46,51,55,60},
			{21,34,37,47,50,56,59,61},
			{35,36,48,49,57,58,62,63}
	};

	for (i = 0; i < H; i+=8){
		for (j = 0; j < W; j+=8, dc_index ++){
			YUV[i][j].Y = dc[dc_index];
			for (int x = 0 ; x < 8; x ++){
				for (int y = 0 ; y < 8; y++){
					if (x != 0 || y != 0){
						YUV[i+x][j+y].Y = AC_com[dc_index][table[x][y]-1];
					}
				}
			}
		}
	}

	bound[0] = dc_index;
	for (i = 0; i < H; i+=8){
		for (j = 0; j < W; j+=8, dc_index++){
			YUV[i][j].U = dc[dc_index];

			for (int x = 0 ; x < 8; x ++){
				for (int y = 0 ; y < 8; y++){
					if (x != 0 || y != 0){
						YUV[i+x][j+y].U = AC_com[dc_index][table[x][y]-1];
					}
				}
			}
		}
	}


	bound[1] = dc_index;
	for (i = 0; i < H; i+=8){
		for (j = 0; j < W; j+=8, dc_index++){
			YUV[i][j].V = dc[dc_index];
			for (int x = 0 ; x < 8; x ++){
				for (int y = 0 ; y < 8; y++){
					if (x != 0 || y != 0){
						YUV[i+x][j+y].V = AC_com[dc_index][table[x][y]-1];
					}
				}
			}
		}
	}
	printf("\n\ndc_index %d",dc_index);
	bound[2] = dc_index;
}


void organize_DC(ImgYUV **YUV, short *dc, short **AC_com, short height, short width, int *bound){
	short H = height * 8, W = width * 8, i = 0 , j = 0;
	int dc_index = 0;
	for (i = 0; i < H; i+=8){
		for (j = 0; j < W; j+=8, dc_index ++){
			YUV[i][j].Y = dc[dc_index];
		}
	}
	for (i = 0; i < H; i+=8){
		for (j = 0; j < W; j+=8, dc_index ++){
			YUV[i][j].U = dc[dc_index];
		}
	}

	for (i = 0; i < H; i+=8){
		for (j = 0; j < W; j+=8, dc_index ++){
			YUV[i][j].V = dc[dc_index];
		}
	}
}

void unquan(ImgYUV **YUV, short frame, short frame1){
	short qua_array[8][8] = {
			{16,11,10,16,24,40,51,61},
			{12,12,14,19,26,58,60,55},
			{14,13,16,24,40,57,69,56},
			{14,17,22,29,51,87,80,62},
			{18,22,37,56,68,109,103,77},
			{24,35,55,64,81,104,113,92},
			{49,64,78,87,103,121,120,101},
			{72,92,95,98,112,100,103,99}
	};
	for (int i = 0 ; i < frame; i ++){
		for (int j = 0 ; j <frame1; j++){
			for (int x = 0 ; x < 8; x ++){
				for (int y = 0 ; y < 8 ; y++){
					YUV[x + (8 * i)][y + (8 * j)].Y = YUV[x + (8 * i)][y + (8 * j)].Y * qua_array[x][y];
					YUV[x + (8 * i)][y + (8 * j)].U = YUV[x + (8 * i)][y + (8 * j)].U * qua_array[x][y];
					YUV[x + (8 * i)][y + (8 * j)].V = YUV[x + (8 * i)][y + (8 * j)].V * qua_array[x][y];
				}
			}
		}
	}
}



// inverse discrete cosine transform
// ¿W≤v∂b¬‡¶^Æ…∞Ï
void IDCT(ImgYUV **YUV, short N, ImgYUV ** YUV_out, short frame, short frame1){
	float co[8][8] ={
			{0.125, 0.1767,0.1767,0.1767,0.1767,0.1767,0.1767,0.1767},
			{0.1767, 0.25,0.25,0.25,0.25,0.25,0.25,0.25},
			{0.1767, 0.25,0.25,0.25,0.25,0.25,0.25,0.25},
			{0.1767, 0.25,0.25,0.25,0.25,0.25,0.25,0.25},
			{0.1767, 0.25,0.25,0.25,0.25,0.25,0.25,0.25},
			{0.1767, 0.25,0.25,0.25,0.25,0.25,0.25,0.25},
			{0.1767, 0.25,0.25,0.25,0.25,0.25,0.25,0.25},
			{0.1767, 0.25,0.25,0.25,0.25,0.25,0.25,0.25}
	};
	float buff[4096];
	for (int l1 = 0,index =0 ; l1 < 8; l1++){
		for (int l2 = 0 ; l2< 8; l2++){
			for (int l3 = 0 ; l3 < 8; l3++){
				for (int l4 = 0 ; l4< 8; l4++,index++){
					buff[index] =  cos(((2*l1 + 1) * l3 * PI)/(2*N)) * cos(((2*l2 + 1) * l4 * PI)/(2*N));
				}
			}
		}
	}
	float temp, temp_1, temp_2;
	short index =0 ;
	for (int f = 0; f < frame; f++){
		for (int f1 = 0; f1 < frame1; f1++,index =0){
			for (int i = 0 ; i < N; i++ ){
				for (int j = 0 ; j < N; j++){
					temp = 0;
					temp_1 = 0;
					temp_2 = 0;
					for (int x = 0; x < N ; x++){
						for(int y = 0 ; y < N; y++, index++){
							temp += co[x][y] * (float)(YUV[x + (8 * f)][y + (8 * f1)].Y ) * buff[index];
							temp_1 += co[x][y] * (float)(YUV[x + (8 * f)][y + (8 * f1)].U ) * buff[index];
							temp_2 += co[x][y] * (float)(YUV[x + (8 * f)][y + (8 * f1)].V ) * buff[index];
						}
					}
					YUV_out[i + (8 * f)][j + (8 *f1)].Y = (short)round(temp) + 128;
					YUV_out[i + (8 * f)][j + (8 *f1)].U = (short)round(temp_1) + 128;
					YUV_out[i + (8 * f)][j + (8 *f1)].V = (short)round(temp_2) + 128;
				}
			}
		}
	}
}


// trasnform from XYZ format back to RGB format(color space)
void color_space(ImgYUV **YUV, ImgRGB **RGB, short H, short W){
	short temp =0;
	for (int i = 0 ; i < H ; i++){
		for(int j = 0; j < W ; j++){
			temp = round (YUV[i][j].Y * 1 + YUV[i][j].U * (-0.0009283) + YUV[i][j].V * (1.4017));
			if (temp > 255){      // overflow might happen during calculation   so value will be set to boundary if it overflows
 				temp = 255;
			}
			if (temp < 0 ){       // overflow might happen during calculation    so value will be set to boundary if it overflows
				temp = 0;
			}
			RGB[i][j].R = temp;
			temp = round (YUV[i][j].Y * (1) + YUV[i][j].U * (-0.34431) + YUV[i][j].V * (-0.71369));
			if (temp > 255){
				temp = 255;
			}
			if (temp < 0 ){
				temp = 0;
			}
			RGB[i][j].G = temp;
			temp = round (YUV[i][j].Y * (1) + YUV[i][j].U * (1.7753) + YUV[i][j].V * (-0.0014964));
			if (temp > 255){
				temp = 255;
			}
			if (temp < 0 ){
				temp = 0;
			}
			RGB[i][j].B = temp;

		}
	}
}
// writing data, this function was made by prof
void output_bmp(ImgRGB **RGB, FILE* outfile, Bitmap bmpheader, int skip){

	char skip_buf[3] = { 0, 0, 0 };
	int x, y;
	fwrite(&bmpheader.identifier, sizeof(short), 1, outfile);
	fwrite(&bmpheader.filesize, sizeof(int), 1, outfile);
	fwrite(&bmpheader.reserved, sizeof(short), 1, outfile);
	fwrite(&bmpheader.reserved2, sizeof(short), 1, outfile);
	fwrite(&bmpheader.bitmap_dataoffset, sizeof(int), 1, outfile);
	fwrite(&bmpheader.bitmap_headersize, sizeof(int), 1, outfile);
	fwrite(&bmpheader.width, sizeof(int), 1, outfile);
	fwrite(&bmpheader.height, sizeof(int), 1, outfile);
	fwrite(&bmpheader.planes, sizeof(short), 1, outfile);
	fwrite(&bmpheader.bits_perpixel, sizeof(short), 1, outfile);
	fwrite(&bmpheader.compression, sizeof(int), 1, outfile);
	fwrite(&bmpheader.bitmap_datasize, sizeof(int), 1, outfile);
	fwrite(&bmpheader.hresolution, sizeof(int), 1, outfile);
	fwrite(&bmpheader.vresolution, sizeof(int), 1, outfile);
	fwrite(&bmpheader.usedcolors, sizeof(int), 1, outfile);
	fwrite(&bmpheader.importantcolors, sizeof(int), 1, outfile);

	for (x = 0; x<bmpheader.height; x++){
		for (y = 0; y<bmpheader.width; y++){
			fwrite(&RGB[x][y].B, sizeof(char), 1, outfile);
			fwrite(&RGB[x][y].G, sizeof(char), 1, outfile);
			fwrite(&RGB[x][y].R, sizeof(char), 1, outfile);
		}
		if (skip != 0) {fwrite(skip_buf, skip, 1, outfile);
		}

		if (x<9) printf("e"); // DEBUG
	}


}

int main(int argc, char **argv)
{
	char* IN = argv[1];   //importing data from files
	char* OUT = argv[2];

	FILE *fp_in;
	FILE *fp_out;
	FILE *JPEG_binary;
	FILE *JPEG_header;



	fp_in = fopen(IN, "rb");
	if (fp_in) printf("Open %s as input file!\n", IN);
	else printf("Fail to open %s as input file!\n", IN);
	fp_out = fopen(OUT, "wb");
	if (fp_out) printf("Open %s as output file!\n", OUT);
	else printf("Fail to open %s as output file!\n", OUT);
	JPEG_binary = fopen("compression.txt", "wb+");
	if(JPEG_binary) printf("open JPEG_binary as binary compression file success");
	else printf("fail to open JPEG_binary as binary compression file ");
	JPEG_header = fopen("JPEG_header.txt", "wb+");
	if(JPEG_header) printf("open JPEG_header as binary compression file success");
	else printf("fail to open JPEG_header as binary compression file ");

	Bitmap bmpheader;
	readheader(fp_in, &bmpheader);  //reading header, the function was from prof
	printf("\n identifier %d \n", bmpheader.identifier);
	printf("\n filesize %d \n", bmpheader.filesize);
	printf("\n bmpheader.reserved %d \n", bmpheader.reserved);
	printf("\nbmpheader.reserved2 %d \n", bmpheader.reserved2);
	printf("\nbitmap_dataoffset %d \n", bmpheader.bitmap_dataoffset);
	printf("\nbmpheader.bitmap_headersize %d \n",bmpheader.bitmap_headersize);
	printf("\nbmpheader.width %d \n", bmpheader.width);
	printf("\nbmpheader.height %d \n", bmpheader.height);
	printf("\nbmpheader.bits_perpixel %d \n", bmpheader.bits_perpixel);
	printf("\nbmpheader.compression %d\n", bmpheader.compression);
	printf("\nbmpheader.bitmap_datasize%d %d\n", bmpheader.bitmap_datasize);
	printf("\nbmpheader.hresolution %d \n", bmpheader.hresolution);
	printf("\nbmpheader.vresolution %d \n",bmpheader.vresolution);
	printf("\nbmpheader.width %d \n", bmpheader.width);
	printf("\nbmpheader.height %d \n", bmpheader.height);
	printf("\nbmpheader.usedcolors %d \n", bmpheader.usedcolors);
	printf("\nbmpheader.importantcolors %d\n", bmpheader.importantcolors);

	short sam;
	int *pt = NULL;
	pt =allocating_memory_int(4);
	pt[0] = 0;

	int H = bmpheader.height;
	int W = bmpheader.width;

	int skip = (4 - (bmpheader.width * 3) % 4);
	if (skip == 4) skip = 0;
	int skip1 = (4 - (bmpheader.height * 3) % 4);
	if (skip1 == 4) skip1 = 0;


	int block_H = H / 8;
	if (H % 8 != 0) block_H = round(block_H);
	int block_W = W / 8;
	if (W % 8 != 0) block_W = round(block_W);

	// declaring some 2d array
	ImgRGB **Data_RGB = malloc_2D(bmpheader.height, bmpheader.width);
	ImgYUV **Data_YUV = malloc_2D_1(bmpheader.height, bmpheader.width);
	ImgYUV **Data_YUV_out = malloc_2D_1(bmpheader.height, bmpheader.width);
	ImgYUV **Data = malloc_2D_1(bmpheader.height, bmpheader.width);
	ImgYUV **buffer = malloc_2D_1(bmpheader.height, bmpheader.width);
	ImgRGB **result = malloc_2D(bmpheader.height, bmpheader.width);
	ImgRGB **test = malloc_2D(bmpheader.height, bmpheader.width);
	ImgYUV ***zigzag_YUV = malloc_3D(block_H, block_W, 64);
	short **AC_com = malloc_2D_short( block_H * block_W * 3, 63);
	ImgYUV *difference = NULL;
	difference = allocating_memory(block_H * block_W * 3);
	short *dc = NULL;
	dc = allocating_memory_short(block_H * block_W * 3);
	char *store = NULL;
	store = allocating_memory_char(bmpheader.height*bmpheader.width * 30);
	memset(store,'\0', bmpheader.height*bmpheader.width * 30);
	AC_table *ta = NULL;
	ta = allocating_memory_AC(100);
	short *buf = NULL;
	buf = allocating_memory_short(63);
	int *set = NULL;
	set =allocating_memory_int(4);
	int *set1 = NULL;
	set1 =allocating_memory_int(3);
	char *st = NULL;
	st = allocating_memory_char(bmpheader.height * bmpheader.width * 30);
	memset(st,'\0', bmpheader.height*bmpheader.width * 30);
	// these variables are used to check for my calculation processes   it has nothing to do with all the program
	// i'll just leave the checking process for references
	short bo1 = 0;
	short bo2 = 0;
	short b1 = 8;
	short b2 = 8;

	InputData(fp_in, Data_RGB, bmpheader.height, bmpheader.width, skip);
	// the rest of printing process is the example of how my calculations were done
	printf("input\n");
	for (int q =  bo1; q < b1 ; q++){
		for (int w = bo2 ; w < b2 ; w++){
			printf("%d ", Data_RGB[q][w].R );
		}
		printf("\n");
	}

	printf("\n");
	// calling function one by one
	YUV_cal(Data_YUV, Data_RGB, H - skip1, W - skip);
	free(Data_RGB);

	printf("Y U V\n");
	for (int q =  bo1; q < b1 ; q++){
		for (int w = bo2 ; w < b2 ; w++){
			printf("%d ", Data_YUV[q][w].Y );
		}
		printf("\n");
	}
	printf("\n");
	for (int q =  bo1; q < b1 ; q++){
		for (int w = bo2 ; w < b2 ; w++){
			printf("%d ", Data_YUV[q][w].U );
		}
		printf("\n");
	}
	printf("\n");
	for (int q =  bo1; q < b1 ; q++){
		for (int w = bo2 ; w < b2 ; w++){
			printf("%d ", Data_YUV[q][w].V );
		}
		printf("\n");
	}
	printf("\n");



	DCT(Data_YUV, 8, Data_YUV_out, block_H, block_W);
	printf("DCT for Y\n");
	for (int q =  bo1; q < b1 ; q++){
		for (int w = bo2 ; w < b2 ; w++){
			printf("%d ", Data_YUV_out[q][w].Y );
		}
		printf("\n");
	}printf("\n");

	free(Data_YUV);


	quan (Data_YUV_out, block_H, block_W);
	printf("quantization for Y\n");

	for (int q =  bo1; q < b1 ; q++){
		for (int w = bo2 ; w < b2 ; w++){
			printf("%d ", Data_YUV_out[q][w].Y );
		}
		printf("\n");
	}




	zigzag(Data_YUV_out, zigzag_YUV, block_H, block_W);
	printf("\nzigzag for Y\n");
	for (int q =  1; q < 64 ; q++){
		printf("%d ", zigzag_YUV[0][0][q].Y );
		if (q/8 == 0 && q >=8)
			printf("\n");
	}
	printf("\n");

	dif(difference, block_H, block_W, zigzag_YUV);



	printf("encoding processes start");
	for (int i = 0, de = 0; i < block_H; i++){
		for (int j = 0 ; j < block_W; j++){
			DCCODE(store, difference[de].Y);
			for (int x= 1 ; x < 64;x++){
				buf[x-1] = zigzag_YUV[i][j][x].Y;
			}
			ACC(buf,ta);
			ACCODE(store,ta);
			de++;
		}
	}


	for (int i = 0, de = 0; i < block_H; i++){
		for (int j = 0 ; j < block_W; j++){
			DCCODE(store, difference[de].U);
			for (int x= 1 ; x < 64;x++){
				buf[x-1] = zigzag_YUV[i][j][x].U;
			}
			ACC(buf,ta);
			ACCODE(store,ta);
			de++;
		}
	}

	for (int i = 0, de = 0; i < block_H; i++){
		for (int j = 0 ; j < block_W; j++){
			DCCODE(store, difference[de].V);
			for (int x= 1 ; x < 64;x++){
				buf[x-1] = zigzag_YUV[i][j][x].V;
			}
			ACC(buf,ta);
			ACCODE(store,ta);
			de++;
		}
	}
	free(zigzag_YUV);
	printf("\nencoding processes finish\n");


	printf("\nwriting file\n");
	digit_write(store, JPEG_binary, set, JPEG_header);
	printf("\nwriting file success\n");


	printf("\nreading file");
	digit_read(JPEG_binary,st,JPEG_header);
	sam = memcmp(store,st,strlen(st));
	if (sam == 0 )printf("\nreading file success\n");
	else printf("\nreading file fails\n");
	printf("\n%d\n %d", strlen(st), strlen(store));
	free(store);


	de(st, pt, dc, AC_com);
	printf("\n decoding starts \n decoding Y result\n");
	for(int i = 0 ; i < 63; i++){
		printf("%d ",AC_com[0][i]);
	}

	free(st);



	for (int i = 1 ; i <block_H * block_W ; i++){
		difference[i].Y = difference[i].Y + difference[i-1].Y;
	}
	for (int i = 1 ; i <block_H * block_W ; i++){
			difference[i].U = difference[i].U + difference[i-1].U;
		}
	for (int i = 1 ; i <block_H * block_W ; i++){
			difference[i].V = difference[i].V + difference[i-1].V;
		}

	for (int i = 1 ; i <block_H * block_W ; i++){
		dc[i] = dc[i] + dc[i-1];
	}
	for (int i = block_H * block_W + 1; i < block_H * block_W * 2; i++){
		dc[i] = dc[i] + dc[i-1];
	}
	for (int i = block_H * block_W * 2 + 1; i < block_H * block_W * 3; i++){
			dc[i] = dc[i] + dc[i-1];
		}



	organize(Data,dc, AC_com,block_H, block_W, set1);

	printf("\norganize for Y\n ");
	for (int q =  bo1; q < b1 ; q++){
		for (int w = bo2 ; w < b2 ; w++){
			printf("%d ", Data[q][w].Y);
		}
		printf("\n");
	}


	printf("\n this is DC values after organization \n");
	for (int q =  0; q < 64 ; q+=8){
		for (int w = 0 ; w < 64 ; w+=8){
			printf("%d ",  Data[q][w].V);
		}
		printf("\n");
	}
	printf("\n the following is correct if up and down is correct it successfully decoded \n");
	for (int q =  0; q < 64 ; q+=8){
		for (int w = 0 ; w < 64 ; w+=8){
			printf("%d ",  Data_YUV_out[q][w].V);
		}
		printf("\n");
	}
	free(Data_YUV_out);

	unquan (Data , block_H, block_W);
	printf("UNQUAN\n");
	for (int q =  bo1; q < b1 ; q++){
		for (int w = bo2 ; w < b2 ; w++){
			printf("%d  ", Data [q][w].Y);
		}
		printf("\n");
	}


	IDCT(Data, 8, buffer, block_H, block_W);


	printf("IDCT\n");
	for (int q =  bo1; q < b1 ; q++){
		for (int w = bo2 ; w < b2 ; w++){
			printf("%d ", buffer[q][w].Y );
		}
		printf("\n");
	}


	color_space(buffer, result, H - skip1, W - skip);
	output_bmp(result, fp_out, bmpheader, skip);   // writing the result to the file

	printf("result\n");
	for (int q =  bo1; q < b1 ; q++){
		for (int w = bo2 ; w < b2 ; w++){
			printf("%d ", result[q][w].R );
		}
		printf("\n");
	}
	printf("\n");
	for (int q =  bo1; q < b1 ; q++){
		for (int w = bo2 ; w < b2 ; w++){
			printf("%d ", result[q][w].G );
		}
		printf("\n");
	}
	printf("\n");
	for (int q =  bo1; q < b1 ; q++){
		for (int w = bo2 ; w < b2 ; w++){
			printf("%d ", result[q][w].B );
		}
		printf("\n");
	}




	fclose(fp_in);
	fclose(fp_out);  // close the file
	printf("jpeg process complete\n");

	free(buffer);
	free(result);
	free(difference);
	return 0;
}

