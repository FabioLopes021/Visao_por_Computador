//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLITÉCNICO DO CÁVADO E DO AVE
//                          2022/2023
//             ENGENHARIA DE SISTEMAS INFORMÁTICOS
//                    VISÃO POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Desabilita (no MSVC++) warnings de funções não seguras (fopen, sscanf, etc...)
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include "vc.h"
#include <math.h>

/*
int valores[7][6] = {
	{151, 234, 13, 100, 24, 100},   //azul
	//{35, 147, 0, 41, 0, 44},    //preto
	{35, 360, 0, 28, 0, 39},    //preto , talvez aumentar mais um pouco (ver depois)
	{0, 26, 0, 61, 0, 48},   //castanho
	{350, 10, 0, 100, 60, 80},   //vermelho
	{5, 16, 60, 100, 80, 100},   //laranja
	{80, 107, 30, 100, 30, 100},    //verde
	{0, 360, 0, 29, 50, 100}    //resistencia
};*/

int valores[7][6] = {
	{151, 234, 13, 100, 24, 100},   //azul
	//{35, 147, 0, 41, 0, 44},    //preto
	{0, 360, 0,100,0,30},    //preto , talvez aumentar mais um pouco (ver depois)
	//{7, 31, 20, 100, 10, 52},   //castanho
	{8, 29, 0, 70, 14, 51},
	{340, 20, 55, 70, 50, 76},   //vermelho
	{7, 16, 65, 95, 80, 95},   //laranja
	{80, 107, 30, 100, 30, 100},    //verde
	{0, 360, 0, 29, 50, 100}   //resistencia
};

//Valores em ohms das bandas de cores relativamente a sua posi�ao
int table[6][3] = {
	{6, 6, 1000000},   //azul
	{0, 0, 1},    //preto 
	{1, 1, 10},   //castanho
	{2, 2, 100},   //vermelho
	{3, 3, 1000},   //laranja
	{5, 5, 100000},    //verde
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//            FUNÇÕES: ALOCAR E LIBERTAR UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


// Alocar memória para uma imagem
IVC* vc_image_new(int width, int height, int channels, int levels)
{
	IVC* image = (IVC*)malloc(sizeof(IVC));

	if (image == NULL) return NULL;
	if ((levels <= 0) || (levels > 255)) return NULL;

	image->width = width;
	image->height = height;
	image->channels = channels;
	image->levels = levels;
	image->bytesperline = image->width * image->channels;
	image->data = (unsigned char*)malloc(image->width * image->height * image->channels * sizeof(char));

	if (image->data == NULL)
	{
		return vc_image_free(image);
	}

	return image;
}


// Libertar memória de uma imagem
IVC* vc_image_free(IVC* image)
{
	if (image != NULL)
	{
		if (image->data != NULL)
		{
			free(image->data);
			image->data = NULL;
		}

		free(image);
		image = NULL;
	}

	return image;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    FUNÇÕES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


char* netpbm_get_token(FILE* file, char* tok, int len)
{
	char* t;
	int c;

	for (;;)
	{
		while (isspace(c = getc(file)));
		if (c != '#') break;
		do c = getc(file);
		while ((c != '\n') && (c != EOF));
		if (c == EOF) break;
	}

	t = tok;

	if (c != EOF)
	{
		do
		{
			*t++ = c;
			c = getc(file);
		} while ((!isspace(c)) && (c != '#') && (c != EOF) && (t - tok < len - 1));

		if (c == '#') ungetc(c, file);
	}

	*t = 0;

	return tok;
}


long int unsigned_char_to_bit(unsigned char* datauchar, unsigned char* databit, int width, int height)
{
	int x, y;
	int countbits;
	long int pos, counttotalbytes;
	unsigned char* p = databit;

	*p = 0;
	countbits = 1;
	counttotalbytes = 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = width * y + x;

			if (countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//*p |= (datauchar[pos] != 0) << (8 - countbits);

				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				*p |= (datauchar[pos] == 0) << (8 - countbits);

				countbits++;
			}
			if ((countbits > 8) || (x == width - 1))
			{
				p++;
				*p = 0;
				countbits = 1;
				counttotalbytes++;
			}
		}
	}

	return counttotalbytes;
}


void bit_to_unsigned_char(unsigned char* databit, unsigned char* datauchar, int width, int height)
{
	int x, y;
	int countbits;
	long int pos;
	unsigned char* p = databit;

	countbits = 1;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = width * y + x;

			if (countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//datauchar[pos] = (*p & (1 << (8 - countbits))) ? 1 : 0;

				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				datauchar[pos] = (*p & (1 << (8 - countbits))) ? 0 : 1;

				countbits++;
			}
			if ((countbits > 8) || (x == width - 1))
			{
				p++;
				countbits = 1;
			}
		}
	}
}


IVC* vc_read_image(char* filename)
{
	FILE* file = NULL;
	IVC* image = NULL;
	unsigned char* tmp;
	char tok[20];
	long int size, sizeofbinarydata;
	int width, height, channels;
	int levels = 255;
	int v;

	// Abre o ficheiro
	if ((file = fopen(filename, "rb")) != NULL)
	{
		// Efectua a leitura do header
		netpbm_get_token(file, tok, sizeof(tok));

		if (strcmp(tok, "P4") == 0) { channels = 1; levels = 1; }	// Se PBM (Binary [0,1])
		else if (strcmp(tok, "P5") == 0) channels = 1;				// Se PGM (Gray [0,MAX(level,255)])
		else if (strcmp(tok, "P6") == 0) channels = 3;				// Se PPM (RGB [0,MAX(level,255)])
		else
		{
#ifdef VC_DEBUG
			printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM, PGM or PPM file.\n\tBad magic number!\n");
#endif

			fclose(file);
			return NULL;
		}

		if (levels == 1) // PBM
		{
			if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM file.\n\tBad size!\n");
#endif

				fclose(file);
				return NULL;
			}

			// Aloca memória para imagem
			image = vc_image_new(width, height, channels, levels);
			if (image == NULL) return NULL;

			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height;
			tmp = (unsigned char*)malloc(sizeofbinarydata);
			if (tmp == NULL) return 0;

#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

			if ((v = fread(tmp, sizeof(unsigned char), sizeofbinarydata, file)) != sizeofbinarydata)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif

				vc_image_free(image);
				fclose(file);
				free(tmp);
				return NULL;
			}

			bit_to_unsigned_char(tmp, image->data, image->width, image->height);

			free(tmp);
		}
		else // PGM ou PPM
		{
			if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &levels) != 1 || levels <= 0 || levels > 255)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PGM or PPM file.\n\tBad size!\n");
#endif

				fclose(file);
				return NULL;
			}

			// Aloca memória para imagem
			image = vc_image_new(width, height, channels, levels);
			if (image == NULL) return NULL;

#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

			size = image->width * image->height * image->channels;

			if ((v = fread(image->data, sizeof(unsigned char), size, file)) != size)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif

				vc_image_free(image);
				fclose(file);
				return NULL;
			}
		}

		fclose(file);
	}
	else
	{
#ifdef VC_DEBUG
		printf("ERROR -> vc_read_image():\n\tFile not found.\n");
#endif
	}

	return image;
}


int vc_write_image(char* filename, IVC* image)
{
	FILE* file = NULL;
	unsigned char* tmp;
	long int totalbytes, sizeofbinarydata;

	if (image == NULL) return 0;

	if ((file = fopen(filename, "wb")) != NULL)
	{
		if (image->levels == 1)
		{
			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height + 1;
			tmp = (unsigned char*)malloc(sizeofbinarydata);
			if (tmp == NULL) return 0;

			fprintf(file, "%s %d %d\n", "P4", image->width, image->height);

			totalbytes = unsigned_char_to_bit(image->data, tmp, image->width, image->height);
			printf("Total = %ld\n", totalbytes);
			if (fwrite(tmp, sizeof(unsigned char), totalbytes, file) != totalbytes)
			{
#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif

				fclose(file);
				free(tmp);
				return 0;
			}

			free(tmp);
		}
		else
		{
			fprintf(file, "%s %d %d 255\n", (image->channels == 1) ? "P5" : "P6", image->width, image->height);

			if (fwrite(image->data, image->bytesperline, image->height, file) != image->height)
			{
#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif

				fclose(file);
				return 0;
			}
		}

		fclose(file);

		return 1;
	}

	return 0;
}



int VC_gray_negative(IVC* srcdst) {
	unsigned char* data = (unsigned char*)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline  = srcdst->bytesperline;
	int channels = srcdst->channels;
	int x, y;
	long int pos;


	//verificaçao de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 1) return 0;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			pos = y * bytesperline + x * channels;
			data[pos] = 255 - data[pos];
		}
	}

	return 1;
}


int VC_RBG_negative(IVC* srcdst) {
	unsigned char* data = (unsigned char*)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->width * srcdst->channels;
	int channels = srcdst->channels;
	int x, y;
	long int pos;


	//verificaçao de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 3) return 0;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			pos = y * bytesperline + x * channels;

			data[pos] = 255 - data[pos];
			data[pos + 1] = 255 - data[pos + 1];
			data[pos + 2] = 255 - data[pos + 2];
		}
	}

	return 1;
}




int VC_rgb_to_gray(IVC* src, IVC* dst){
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char *datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;
	float rf, gf, bf;

	//verificaçao de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 3) || (dst->channels != 1)) return 0;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			rf = (float)datasrc[pos_src];
			gf = (float)datasrc[pos_src + 1];
			bf = (float)datasrc[pos_src + 2];

			datadst[pos_dst] = (unsigned char)((rf * 0.299) + (gf * 0.587) + (bf * 0.114));
		}
	}

	return 1;
}



int vc_rgb_to_hsv(IVC* src, IVC* dst) {
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;
	float r, g, b;

	//verificaçao de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 3) || (dst->channels != 3)) return 0;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			r = (float)datasrc[pos_src];
			g = (float)datasrc[pos_src + 1];
			b = (float)datasrc[pos_src + 2];

			int max = MAX3(r, g, b);
			int min = MIN3(r, g, b);

			//Calculo de Value
			float value = max;

			//Calculo de H
			float hue;
			if (max == r) {
				if (g >= b) {
					hue = 60 * (g - b) / (max - min);
				}
				else {
					hue = 360 + 60 * (g - b) / (max - min);
				}
			}
			else {
				if (max == g) {
					hue = 120 + 60 * (b - r) / (max - min);
				}
				if (max == b) {
					hue = 240 + 60 * (r - g) / (max - min);
				}
			}
			hue = hue / 360 * 255;

			//Calculo da saturaçao 
			float sat;
			if (max == 0 || max == min)
				sat = 0;
			else
			{
				sat = (max - min) / value * 255;
			}


			datadst[pos_dst] = hue;
			datadst[pos_dst + 1] = sat;
			datadst[pos_dst + 2] = value;
		}
	}

	return 1;
}



int vc_rgb_to_hsv_OCV(IVC* src, IVC* dst) {
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;
	float r, g, b;

	//verificaçao de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 3) || (dst->channels != 3)) return 0;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			r = (float)datasrc[pos_src + 2];
			g = (float)datasrc[pos_src + 1];
			b = (float)datasrc[pos_src];

			int max = MAX3(r, g, b);
			int min = MIN3(r, g, b);

			//Calculo de Value
			float value = max;

			//Calculo de H
			float hue;
			if (max == r) {
				if (g >= b) {
					hue = 60 * (g - b) / (max - min);
				}
				else {
					hue = 360 + 60 * (g - b) / (max - min);
				}
			}
			else {
				if (max == g) {
					hue = 120 + 60 * (b - r) / (max - min);
				}
				if (max == b) {
					hue = 240 + 60 * (r - g) / (max - min);
				}
			}
			hue = hue / 360 * 255;

			//Calculo da saturaçao 
			float sat;
			if (max == 0 || max == min)
				sat = 0;
			else
			{
				sat = (max - min) / value * 255;
			}


			datadst[pos_dst] = hue;
			datadst[pos_dst + 1] = sat;
			datadst[pos_dst + 2] = value;
		}
	}

	return 1;
}






int vc_hsv_segmentation(IVC* src, IVC* dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax) {
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;
	float h, s, v;
	

	//verificaçao de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 3) || (dst->channels != 1)) return 0;
	if (hmin < 0 || hmax > 360 || smin < 0 || smax > 100 || vmin < 0 || vmax > 100)    return 0; // valores que nos da no GIMP


	unsigned char new_hmin, new_hmax, new_smin, new_smax, new_vmin, new_vmax;

	new_hmin = (unsigned char)((float)hmin / 360 * 255);
	new_hmax = (unsigned char)((float)hmax / 360 * 255);
	new_smin = (unsigned char)((float)smin / 100 * 255);
	new_smax = (unsigned char)((float)smax / 100 * 255);
	new_vmin = (unsigned char)((float)vmin / 100 * 255);
	new_vmax = (unsigned char)((float)vmax / 100 * 255);


	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			h = datasrc[pos_src];
			s = datasrc[pos_src + 1];
			v = datasrc[pos_src + 2];

			if (hmin > hmax) {
				if (!(h > new_hmax && h < new_hmin) && s >= new_smin && s <= new_smax && v >= new_vmin && v <= new_vmax)
					dst->data[pos_dst] = (unsigned char)255;
				else 
					dst->data[pos_dst] = (unsigned char)0;
			}
			else {
				if (h >= new_hmin && h <= new_hmax && s >= new_smin && s <= new_smax && v >= new_vmin && v <= new_vmax) {
					dst->data[pos_dst] = (unsigned char)255;
				}
				else dst->data[pos_dst] = (unsigned char)0;
			}
			
			
		}
	}

	return 1;
}


int vc_scale_gray_to_rgb(IVC* src, IVC* dst)
{
	int x, y;
	long int pos_src, pos_dst;
	float r, g, b;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 3)) return 0;

	for (y = 0; y < src->height; y++)
	{
		for (x = 0; x < src->width; x++)
		{
			pos_src = y * src->bytesperline + x * src->channels;
			pos_dst = y * dst->bytesperline + x * dst->channels;

			if (src->data[pos_src] < 64)
			{
				r = 0;
				g = src->data[pos_src] * 4;
				b = 255;
			}
			else if (src->data[pos_src] >= 192)
			{
				r = 255;
				g = 255 - (src->data[pos_src] - 192) * 4;
				b = 0;
			}
			else if (src->data[pos_src] >= 64 && src->data[pos_src] < 128)
			{
				r = 0;
				g = 255;
				b = 255 - (src->data[pos_src] - 64) * 4;
			}
			else
			{
				r = (src->data[pos_src] - 128) * 4;
				g = 255;
				b = 0;
			}

			dst->data[pos_dst] = (unsigned char)r;
			dst->data[pos_dst + 1] = (unsigned char)g;
			dst->data[pos_dst + 2] = (unsigned char)b;
		}
	}
	return 1;
}


int vc_count_pixeis(IVC* src) {
	unsigned char* data = (unsigned char*)src->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y;
	long int pos;
	int count = 0;

	//verificaçao de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if (channels != 1) return 0;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			pos = y * bytesperline + x * channels;
			if (data[pos] == 255)
				count++;
		}
	}

	return count;
}



int vc_gray_to_binary(IVC* src, IVC* dst, int threshold) {
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int x, y;
	long int pos_src, pos_dst;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))return 0;
	if ((src->width != dst->width) || (src->height != dst->height))return 0;
	if ((src->channels != 1) || (dst->channels != 1))return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			if (datasrc[pos_src] > threshold) {
				datadst[pos_dst] = 255;
			}
			else
			{
				datadst[pos_dst] = 0;
			}

		}
	}

	return 1;
}


int vc_gray_to_binary_global_mean(IVC* src, IVC* dst) {
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int x, y;
	long int pos_src, pos_dst;
	float threshold = 0;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))return 0;
	if ((src->width != dst->width) || (src->height != dst->height))return 0;
	if ((src->channels != 1) || (dst->channels != 1))return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			threshold += datasrc[pos_src];
		}
	}

	threshold = threshold / (width * height);

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			if (datasrc[pos_src] > threshold) {
				datadst[pos_dst] = 255;
			}
			else
			{
				datadst[pos_dst] = 0;
			}

		}
	}

}



int vc_gray_to_binary_midpoint(IVC* src, IVC* dst, int kernel) {
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int x, y;
	int x2, y2;
	long int pos_src, pos_dst;
	long int pos_off;
	float max, min;
	float threshold = 0;
	int offset = (kernel - 1) / 2;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))return 0;
	if ((src->width != dst->width) || (src->height != dst->height))return 0;
	if ((src->channels != 1) || (dst->channels != 1))return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			min = 255;
			max = 0;



			for (y2 = y - offset; y2 <= y + offset; y2++) {
				if (y2 < 0)
					y2 = 0;
				if (y2 > height)
					continue;
				for (x2 = x - offset; x2 <= x + offset; x2++) {
					if (x2 < 0)
						x2 = 0;
					if (x2 > width)
						continue;

					pos_off = y2 * bytesperline_src + x2 * channels_src;

					if (datasrc[pos_off] > max)	max = datasrc[pos_off];
					if (datasrc[pos_off] < min)	min = datasrc[pos_off];
				}
			}

			threshold = 0.5 * (float)(min + max);

			if (datasrc[pos_src] > threshold) {
				datadst[pos_dst] = 255;
			}
			else
			{
				datadst[pos_dst] = 0;
			}

		}
	}

}


int vc_gray_to_binary_bernsen(IVC* src, IVC* dst, int kernel, int cMin) {
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int x, y;
	int x2, y2;
	long int pos_src, pos_dst;
	long int pos_off;
	float max, min;
	float threshold = 0;
	int offset = (kernel - 1) / 2;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))return 0;
	if ((src->width != dst->width) || (src->height != dst->height))return 0;
	if ((src->channels != 1) || (dst->channels != 1))return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			min = 255;
			max = 0;



			for (y2 = y - offset; y2 <= y + offset; y2++) {
				if (y2 < 0)
					y2 = 0;
				if (y2 > height)
					continue;
				for (x2 = x - offset; x2 <= x + offset; x2++) {
					if (x2 < 0)
						x2 = 0;
					if (x2 > width)
						continue;

					pos_off = y2 * bytesperline_src + x2 * channels_src;

					if (datasrc[pos_off] > max)	max = datasrc[pos_off];
					if (datasrc[pos_off] < min)	min = datasrc[pos_off];
				}
			}

			if ((max - min) < cMin)
				threshold = (float)(src->levels)/2.0f;
			else {
				threshold = 0.5 * (float)(min + max);
			}
			

			if (datasrc[pos_src] > threshold) {
				datadst[pos_dst] = 255;
			}
			else
			{
				datadst[pos_dst] = 0;
			}

		}
	}

}


/*
int vc_gray_to_binary_bernsen(IVC* src, IVC* dst, int kernel, float k) {
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int x, y;
	int x2, y2;
	long int pos_src, pos_dst;
	long int pos_off;
	float max, min;
	float threshold = 0;
	int offset = (kernel - 1) / 2;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))return 0;
	if ((src->width != dst->width) || (src->height != dst->height))return 0;
	if ((src->channels != 1) || (dst->channels != 1))return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			min = 255;
			max = 0;



			for (y2 = y - offset; y2 <= y + offset; y2++) {
				if (y2 < 0)
					y2 = 0;
				if (y2 > height)
					continue;
				for (x2 = x - offset; x2 <= x + offset; x2++) {
					if (x2 < 0)
						x2 = 0;
					if (x2 > width)
						continue;

					pos_off = y2 * bytesperline_src + x2 * channels_src;

					if (datasrc[pos_off] > max)	max = datasrc[pos_off];
					if (datasrc[pos_off] < min)	min = datasrc[pos_off];
				}
			}

			if ((max - min) < cMin)
				threshold = (float)(src->levels) / 2.0f;
			else {
				threshold = 0.5 * (float)(min + max);
			}


			if (datasrc[pos_src] > threshold) {
				datadst[pos_dst] = 255;
			}
			else
			{
				datadst[pos_dst] = 0;
			}

		}
	}

}
*/

int vc_binary_dilate(IVC* src, IVC* dst, int kernel) {
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int x, y;
	int y2, x2;
	int limxs, limxf, limys, limyf;
	long int pos_src, pos_dst;
	long int pos_off;
	float max = src->levels, min = 0;
	float threshold = 0;
	int offset = (kernel - 1) / 2;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))return 0;
	if ((src->width != dst->width) || (src->height != dst->height))return 0;
	if ((src->channels != 1) || (dst->channels != 1))return 0;



	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			limys = y - offset;
			limxs = x - offset;
			limyf = y + offset;
			limxf = x + offset;

			if (limys < 0)
				limys = 0;
			if (limxs < 0)
				limxs = 0;
			if (limyf > height)
				limyf = height - 1;
			if (limxf > width)
				limxf = width - 1;

			int flag = 0;

			for (y2 = limys; y2 <= limyf; y2++) {
				for (x2 = limxs; x2 <= limxf; x2++) {
					pos_off = y2 * bytesperline_src + x2 * channels_src;

					if (datasrc[pos_off] == max) {
						flag = 1;
						break;
					}
				}
				if (flag == 1)
					break;
			}

			if (flag == 1)
				datadst[pos_dst] = max;
			else
				datadst[pos_dst] = 0;


		}
	}
	return 1;
}




int vc_binary_erode(IVC* src, IVC* dst, int kernel) {
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int x, y;
	int y2, x2;
	int limxs, limxf, limys, limyf;
	long int pos_src, pos_dst;
	long int pos_off;
	float max = src->levels, min = 0;
	float threshold = 0;
	int offset = (kernel - 1) / 2;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))return 0;
	if ((src->width != dst->width) || (src->height != dst->height))return 0;
	if ((src->channels != 1) || (dst->channels != 1))return 0;



	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			limys = y - offset;
			limxs = x - offset;
			limyf = y + offset;
			limxf = x + offset;

			if (limys < 0)
				limys = 0;
			if (limxs < 0)
				limxs = 0;
			if (limyf > height)
				limyf = height - 1;
			if (limxf > width)
				limxf = width - 1;

			int flag = 0;

			for (y2 = limys; y2 <= limyf; y2++) {
				if (flag == 1)
					break;
				for (x2 = limxs; x2 <= limxf; x2++) {
					pos_off = y2 * bytesperline_src + x2 * channels_src;
					
					if (datasrc[pos_off] == 0) {
						flag = 1;
						break;
					}
				}
			}

			if (flag == 1)
				datadst[pos_dst] = 0;
			else
				datadst[pos_dst] = max;
		}
	}
	return 1;
}


int vc_difference(IVC* src1, IVC* src2, IVC* dst) {
	unsigned char* datasrc = (unsigned char*)src1->data;
	int bytesperline_src = src1->width * src1->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	unsigned char* datasrc1 = (unsigned char*)src1->data;
	unsigned char* datasrc2 = (unsigned char*)src2->data;
	int width = src1->width;
	int height = src1->height;
	int bytesperline = dst->width * dst->channels;
	int channels = dst->channels;
	int x, y;
	long int pos;


	if ((src1->width <= 0) || (src1->height <= 0) || (src1->data == NULL))return 0;
	if ((src2->width <= 0) || (src2->height <= 0) || (src2->data == NULL))return 0;
	if((src1->width != src2->width) || (src1->height != src2->height) || (src1->channels != src2->channels))
	if ((src1->width != dst->width) || (src1->height != dst->height))return 0;
	if ((src1->channels != 1) || (dst->channels != 1))return 0;


	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			pos = y * bytesperline + x * channels;

			datadst[pos] = datasrc1[pos] - datasrc2[pos];
		}
	}

	return 1;
}

int vc_inicialize(IVC* src1, IVC* src2, IVC* dst) {
	unsigned char* datasrc = (unsigned char*)src1->data;
	int bytesperline_src = src1->width * src1->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	unsigned char* datasrc1 = (unsigned char*)src1->data;
	unsigned char* datasrc2 = (unsigned char*)src2->data;
	int width = src1->width;
	int height = src1->height;
	int bytesperline = dst->width * dst->channels;
	int channels = dst->channels;
	int x, y;
	long int pos;


	if ((src1->width <= 0) || (src1->height <= 0) || (src1->data == NULL))return 0;
	if ((src2->width <= 0) || (src2->height <= 0) || (src2->data == NULL))return 0;
	if ((src1->width != src2->width) || (src1->height != src2->height) || (src1->channels != src2->channels))
	if ((src1->width != dst->width) || (src1->height != dst->height))return 0;
	if ((src1->channels != 1) || (dst->channels != 1))return 0;
	

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			pos = y * bytesperline + x * channels;

			if (datasrc2[pos] != 0) {
				datadst[pos] = datasrc1[pos];
			}
			else {
				datadst[pos] = 0;
			}			
		}
	}

	return 1;
}

int vc_binary_open(IVC* src, IVC* dst, int kernel_dil, int kernel_ero) {
	IVC* temporary;

	temporary = vc_image_new(src->width, src->height, src->channels, src->levels);


	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((dst->width <= 0) || (dst->height <= 0) || (dst->data == NULL)) return 0;
	if (temporary == NULL) return 0;
	if ((kernel_dil % 2 == 0) || (kernel_ero % 2 == 0)) return 0;

	if (!vc_binary_erode(src, temporary, kernel_ero));
	if (!vc_binary_dilate(temporary, dst, kernel_dil));

	vc_image_free(temporary);

	return 1;
}


int vc_binary_close(IVC* src, IVC* dst, int kernel_dil, int kernel_ero) {
	IVC* temporary;

	temporary = vc_image_new(src->width, src->height, src->channels, src->levels);


	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((dst->width <= 0) || (dst->height <= 0) || (dst->data == NULL)) return 0;
	if (temporary == NULL) return 0;
	if ((kernel_dil % 2 == 0) || (kernel_ero % 2 == 0)) return 0;
	
	if (!vc_binary_dilate(src, temporary, kernel_dil));
	if (!vc_binary_erode(temporary, dst, kernel_ero));
	
	vc_image_free(temporary);

	return 1;
}


// Etiquetagem de blobs
// src		: Imagem binária de entrada
// dst		: Imagem grayscale (irá conter as etiquetas)
// nlabels	: Endereço de memória de uma variável, onde será armazenado o número de etiquetas encontradas.
// OVC*		: Retorna um array de estruturas de blobs (objectos), com respectivas etiquetas. É necessário libertar posteriormente esta memória.
OVC* vc_binary_blob_labelling(IVC* src, IVC* dst, int* nlabels)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, a, b;
	long int i, size;
	long int posX, posA, posB, posC, posD;
	int labeltable[256] = { 0 };
	int labelarea[256] = { 0 };
	int label = 1; // Etiqueta inicial.
	int num, tmplabel;
	OVC* blobs; // Apontador para array de blobs (objectos) que será retornado desta função.

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return NULL;
	if (channels != 1) return NULL;

	// Copia dados da imagem binária para imagem grayscale
	memcpy(datadst, datasrc, bytesperline * height);

	// Todos os pixéis de plano de fundo devem obrigatóriamente ter valor 0
	// Todos os pixéis de primeiro plano devem obrigatóriamente ter valor 255
	// Serão atribuídas etiquetas no intervalo [1,254]
	// Este algoritmo está assim limitado a 254 labels
	for (i = 0, size = bytesperline * height; i < size; i++)
	{
		if (datadst[i] != 0) datadst[i] = 255;
	}

	// Limpa os rebordos da imagem binária
	for (y = 0; y < height; y++)
	{
		datadst[y * bytesperline + 0 * channels] = 0;
		datadst[y * bytesperline + (width - 1) * channels] = 0;
	}
	for (x = 0; x < width; x++)
	{
		datadst[0 * bytesperline + x * channels] = 0;
		datadst[(height - 1) * bytesperline + x * channels] = 0;
	}

	// Efectua a etiquetagem
	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			// Kernel:
			// A B C
			// D X

			posA = (y - 1) * bytesperline + (x - 1) * channels; // A
			posB = (y - 1) * bytesperline + x * channels; // B
			posC = (y - 1) * bytesperline + (x + 1) * channels; // C
			posD = y * bytesperline + (x - 1) * channels; // D
			posX = y * bytesperline + x * channels; // X

			// Se o pixel foi marcado
			if (datadst[posX] != 0)
			{
				if ((datadst[posA] == 0) && (datadst[posB] == 0) && (datadst[posC] == 0) && (datadst[posD] == 0))
				{
					datadst[posX] = label;
					labeltable[label] = label;
					label++;
				}
				else
				{
					num = 255;

					// Se A está marcado
					if (datadst[posA] != 0) num = labeltable[datadst[posA]];
					// Se B está marcado, e é menor que a etiqueta "num"
					if ((datadst[posB] != 0) && (labeltable[datadst[posB]] < num)) num = labeltable[datadst[posB]];
					// Se C está marcado, e é menor que a etiqueta "num"
					if ((datadst[posC] != 0) && (labeltable[datadst[posC]] < num)) num = labeltable[datadst[posC]];
					// Se D está marcado, e é menor que a etiqueta "num"
					if ((datadst[posD] != 0) && (labeltable[datadst[posD]] < num)) num = labeltable[datadst[posD]];

					// Atribui a etiqueta ao pixel
					datadst[posX] = num;
					labeltable[num] = num;

					// Actualiza a tabela de etiquetas
					if (datadst[posA] != 0)
					{
						if (labeltable[datadst[posA]] != num)
						{
							for (tmplabel = labeltable[datadst[posA]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posB] != 0)
					{
						if (labeltable[datadst[posB]] != num)
						{
							for (tmplabel = labeltable[datadst[posB]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posC] != 0)
					{
						if (labeltable[datadst[posC]] != num)
						{
							for (tmplabel = labeltable[datadst[posC]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posD] != 0)
					{
						if (labeltable[datadst[posD]] != num)
						{
							for (tmplabel = labeltable[datadst[posD]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
				}
			}
		}
	}

	// Volta a etiquetar a imagem
	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			posX = y * bytesperline + x * channels; // X

			if (datadst[posX] != 0)
			{
				datadst[posX] = labeltable[datadst[posX]];
			}
		}
	}

	//printf("\nMax Label = %d\n", label);

	// Contagem do número de blobs
	// Passo 1: Eliminar, da tabela, etiquetas repetidas
	for (a = 1; a < label - 1; a++)
	{
		for (b = a + 1; b < label; b++)
		{
			if (labeltable[a] == labeltable[b]) labeltable[b] = 0;
		}
	}
	// Passo 2: Conta etiquetas e organiza a tabela de etiquetas, para que não hajam valores vazios (zero) entre etiquetas
	*nlabels = 0;
	for (a = 1; a < label; a++)
	{
		if (labeltable[a] != 0)
		{
			labeltable[*nlabels] = labeltable[a]; // Organiza tabela de etiquetas
			(*nlabels)++; // Conta etiquetas
		}
	}

	// Se não há blobs
	if (*nlabels == 0) return NULL;

	// Cria lista de blobs (objectos) e preenche a etiqueta
	blobs = (OVC*)calloc((*nlabels), sizeof(OVC));
	if (blobs != NULL)
	{
		for (a = 0; a < (*nlabels); a++) blobs[a].label = labeltable[a];
	}
	else return NULL;

	return blobs;
}


int vc_binary_blob_info(IVC* src, OVC* blobs, int nblobs)
{
	unsigned char* data = (unsigned char*)src->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, i;
	long int pos;
	int xmin, ymin, xmax, ymax;
	long int sumx, sumy;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if (channels != 1) return 0;

	// Conta área de cada blob
	for (i = 0; i < nblobs; i++)
	{
		xmin = width - 1;
		ymin = height - 1;
		xmax = 0;
		ymax = 0;

		sumx = 0;
		sumy = 0;

		blobs[i].area = 0;

		for (y = 1; y < height - 1; y++)
		{
			for (x = 1; x < width - 1; x++)
			{
				pos = y * bytesperline + x * channels;

				if (data[pos] == blobs[i].label)
				{
					// Área
					blobs[i].area++;

					// Centro de Gravidade
					sumx += x;
					sumy += y;

					// Bounding Box
					if (xmin > x) xmin = x;
					if (ymin > y) ymin = y;
					if (xmax < x) xmax = x;
					if (ymax < y) ymax = y;

					// Perímetro
					// Se pelo menos um dos quatro vizinhos não pertence ao mesmo label, então é um pixel de contorno
					if ((data[pos - 1] != blobs[i].label) || (data[pos + 1] != blobs[i].label) || (data[pos - bytesperline] != blobs[i].label) || (data[pos + bytesperline] != blobs[i].label))
					{
						blobs[i].perimeter++;
					}
				}
			}
		}

		// Bounding Box
		blobs[i].x = xmin;
		blobs[i].y = ymin;
		blobs[i].width = (xmax - xmin) + 1;
		blobs[i].height = (ymax - ymin) + 1;

		// Centro de Gravidade
		//blobs[i].xc = (xmax - xmin) / 2;
		//blobs[i].yc = (ymax - ymin) / 2;
		blobs[i].xc = sumx / MAX(blobs[i].area, 1);
		blobs[i].yc = sumy / MAX(blobs[i].area, 1);
	}

	return 1;
}


int vc_gray_histogram_show(IVC* src, IVC* dst) {
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	int n = width * height;
	int maxpdf = 0;
	int ni[256] = { 0 };
	int pdf[256] = { 0 };

	//Calculo de pdf
	for (int i = 0; i < width * height; ni[datasrc[i++]]++);

	for (int i = 0; i < 255; i++) {
		pdf[i] = (float)ni[i] / (float)n;
	}
	
	for (int i = 0; i < 255; i++) {
		if (maxpdf < pdf[i])
			maxpdf = pdf[i];
	}
	float pdfnorm[256];

	for (int i = 0; i < 255; i++) {
		pdfnorm[i] = pdf[i] / maxpdf;
	}

	for (int i = 0; i < 256 * 256; i++)
		datadst[i] = 0;
	for (int x = 0; x < 256; x++) {
		for (y = (256 - 1); y >= (256 - 1) - pdfnorm[x] * 255; y--) {
			datadst[y * 256 + x] = 255;
		}
	}


	return 1;
}

int vc_gray_edge_prewitt(IVC* src, IVC* dst, float th) {
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline = src->width * src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int channels = dst->channels;
	int x, y;
	long int pos;

	long int posA, posB, posC, posD, posE, posF, posG, posH, posI;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL) || (src->channels != 1)) return 0;
	if ((dst->width <= 0) || (dst->height <= 0) || (dst->data == NULL) || (dst->channels != 1)) return 0;
	if((src->width != dst->width) || (src->height!= dst->height))
	if (th < 0) return 0;

	memset(dst->data, 0, width * height);
	for (y = 1; y < height - 1; y++) {
		for (x = 1; x < width - 1; x++) {
			pos = y * bytesperline + x * channels;

			posA = (y - 1) * bytesperline + (x - 1) * channels;
			posB = (y - 1) * bytesperline + x * channels;
			posC = (y - 1) * bytesperline + (x + 1) * channels;
			posD = y * bytesperline + (x - 1) * channels;
			posE = y * bytesperline + x * channels;
			posF = y * bytesperline + (x + 1) * channels;
			posG = (y + 1) * bytesperline + (x - 1) * channels;
			posH = (y + 1) * bytesperline + x * channels;
			posI = (y + 1) * bytesperline + (x + 1) * channels;

			float mx = 0, my = 0;

			mx += (-1 * datasrc[posA]);
			mx += (0 * datasrc[posB]);
			mx += (1 * datasrc[posC]);
			mx += (-1 * datasrc[posD]);
			mx += (0 * datasrc[posE]);
			mx += (1 * datasrc[posF]);
			mx += (-1 * datasrc[posG]);
			mx += (0 * datasrc[posH]);
			mx += (1 * datasrc[posI]);

			mx = mx / 3;

			my += (-1 * datasrc[posA]);
			my += (-1 * datasrc[posB]);
			my += (-1 * datasrc[posC]);
			my += (0 * datasrc[posD]);
			my += (0 * datasrc[posE]);
			my += (0 * datasrc[posF]);
			my += (1 * datasrc[posG]);
			my += (1 * datasrc[posH]);
			my += (1 * datasrc[posI]);

			my = my / 3;

			float result = sqrtf((mx * mx) + (my * my));
			
			if (result > th) {
				//branco
				datadst[pos] = 255;
			}
			else {
				//preto
				datadst[pos] = 0;
			}
		}
	}



	return 1;
}



int vc_gray_lowpass_mean_filter(IVC* src, IVC* dst, int kernelsize) {
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline = src->width * src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int channels = dst->channels;
	int x, y;
	long int pos;
	int offset = (kernelsize - 1) / 2;
	int y2, x2;
	int limxs, limxf, limys, limyf;
	long int pos_src, pos_dst;
	long int pos_off;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL) || (src->channels != 1)) return 0;
	if ((dst->width <= 0) || (dst->height <= 0) || (dst->data == NULL) || (dst->channels != 1)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height))
	if (kernelsize % 2 == 0) return 0;



	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			int count = 0;
			int soma = 0;
			pos_src = y * bytesperline + x * channels;
			pos_dst = y * bytesperline + x * channels;

			limys = y - offset;
			limxs = x - offset;
			limyf = y + offset;
			limxf = x + offset;

			if (limys < 0)
				limys = 0;
			if (limxs < 0)
				limxs = 0;
			if (limyf > height)
				limyf = height - 1;
			if (limxf > width)
				limxf = width - 1;

			int flag = 0;

			for (y2 = limys; y2 <= limyf; y2++) {
				for (x2 = limxs; x2 <= limxf; x2++) {
					pos_off = y2 * bytesperline + x2 * channels;
					count++;
					soma += datasrc[pos_off];
				}
			}

			datadst[pos_dst] = soma / count;
		}
	}

	return 1;
}


int vc_gray_lowpass_median_filter(IVC* src, IVC* dst, int kernelsize) {
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline = src->width * src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int channels = dst->channels;
	int x, y;
	long int pos;
	int offset = (kernelsize - 1) / 2;
	int y2, x2;
	int limxs, limxf, limys, limyf;
	long int pos_src, pos_dst;
	long int pos_off;
	int* array;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL) || (src->channels != 1)) return 0;
	if ((dst->width <= 0) || (dst->height <= 0) || (dst->data == NULL) || (dst->channels != 1)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height))
	if (kernelsize % 2 == 0) return 0;

	array = (int*)malloc(sizeof(int) * (kernelsize * kernelsize));

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			int count = 0;
			pos_src = y * bytesperline + x * channels;
			pos_dst = y * bytesperline + x * channels;

			limys = y - offset;
			limxs = x - offset;
			limyf = y + offset;
			limxf = x + offset;

			if (limys < 0)
				limys = 0;
			if (limxs < 0)
				limxs = 0;
			if (limyf > height)
				limyf = height - 1;
			if (limxf > width)
				limxf = width - 1;

			for (y2 = limys; y2 <= limyf; y2++) {
				for (x2 = limxs; x2 <= limxf; x2++) {
					pos_off = y2 * bytesperline + x2 * channels;
					array[count] = datasrc[pos_off];
					count++;
				}
			}

			bubbleSort(array, count);
			if (count % 2 == 0) {
				int m1, m2;
				m1 = array[count / 2];
				m2 = array[(count / 2) + 1];
				datadst[pos_dst] = (m1 + m2) / 2;
			}
			else {
				int m;
				m = array[(count / 2) + 1];
				datadst[pos_dst] = m;
			}
		}
	}

	free(array);
	return 1;
}


void bubbleSort(int* array, int n) {
	for (int i = 0; i < n - 1; i++) {
		for (int j = i + 1; j < n; j++) {
			if (array[i] > array[j]) {
				int temp = array[i];
				array[i] = array[j];
				array[j] = temp;
			}
		}
	}
}

int drawRectangle(IVC* srcdst, int xtl, int ytl, int xbr, int ybr, int r, int g, int b) {
	unsigned char* datasrc = (unsigned char*)srcdst->data;
	int bytesperline = srcdst->width * srcdst->channels;
	int width = srcdst->width;
	int height = srcdst->height;
	int channels = srcdst->channels;

	if ((width <= 0) || (height <= 0) || (datasrc == NULL) || (channels != 3)) return 0;
	if (xtl < 0 || xtl >= width || ytl < 0 || ytl >= height) return 0;
	if (xbr < 0 || xbr >= width || ybr < 0 || ybr >= height) return 0;

	// Corrigir ordem das coordenadas se necessário
	if (xtl > xbr) { int tmp = xtl; xtl = xbr; xbr = tmp; }
	if (ytl > ybr) { int tmp = ytl; ytl = ybr; ybr = tmp; }

	// Desenhar borda superior e inferior
	for (int x = xtl; x <= xbr; x++) {
		long int pos_top = ytl * bytesperline + x * channels;
		long int pos_bottom = ybr * bytesperline + x * channels;

		datasrc[pos_top] = b;
		datasrc[pos_top + 1] = g;
		datasrc[pos_top + 2] = r;

		datasrc[pos_bottom] = b;
		datasrc[pos_bottom + 1] = g;
		datasrc[pos_bottom + 2] = r;
	}

	// Desenhar borda esquerda e direita
	for (int y = ytl; y <= ybr; y++) {
		long int pos_left = y * bytesperline + xtl * channels;
		long int pos_right = y * bytesperline + xbr * channels;

		datasrc[pos_left] = b;
		datasrc[pos_left + 1] = g;
		datasrc[pos_left + 2] = r;

		datasrc[pos_right] = b;
		datasrc[pos_right + 1] = g;
		datasrc[pos_right + 2] = r;
	}

	return 1;
}