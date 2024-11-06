//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLITÉCNICO DO CÁVADO E DO AVE
//                          2022/2023
//             ENGENHARIA DE SISTEMAS INFORMÁTICOS
//                    VISÃO POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


#define VC_DEBUG
#define MAX3(r,g,b) (r > g? (r > b ? r : b) : (g > b ? g : b))
#define MIN3(r,g,b) (r < g? (r < b ? r : b) : (g < b ? g : b))
#define MAX(a,b) (a>b?a:b)
#define MIN(a,b) (a<b?a:b)


//---------------------TP---------------------------------

typedef struct {
    int color[3];
    int x[3], y[3];
    int xl, yl, width, height;
    int value;
} cores;

extern int valores[7][6];

extern int table[6][3];

// Definir valores de cores
#define azul 0
#define preto 1
#define castanho 2
#define vermelho 3
#define laranja 4
#define verde 5
#define resistencia 6
#define hMin 0
#define hMax 1
#define sMin 2
#define sMax 3
#define vMin 4
#define vMax 5

//---------------------TP----------------------------------




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   ESTRUTURA DE UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


typedef struct {
	unsigned char* data;
	int width, height;
	int channels;			// Binário/Cinzentos=1; RGB=3
	int levels;				// Binário=1; Cinzentos [1,255]; RGB [1,255]
	int bytesperline;		// width * channels
} IVC;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   ESTRUTURA DE UM BLOB (OBJECTO)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
	int x, y, width, height;	// Caixa Delimitadora (Bounding Box)
	int area;					// Área
	int xc, yc;					// Centro-de-massa
	int perimeter;				// Perímetro
	int label;					// Etiqueta
} OVC;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    PROTÓTIPOS DE FUNÇÕES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

OVC* vc_binary_blob_labelling(IVC* src, IVC* dst, int* nlabels);
int vc_binary_blob_info(IVC* src, OVC* blobs, int nblobs);


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    PROTÓTIPOS DE FUNÇÕES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// FUNÇÕES: ALOCAR E LIBERTAR UMA IMAGEM
IVC* vc_image_new(int width, int height, int channels, int levels);
IVC* vc_image_free(IVC* image);

// FUNÇÕES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
IVC* vc_read_image(char* filename);
int vc_write_image(char* filename, IVC* image);


//Funçao
int VC_gray_negative(IVC* srtdst);
int VC_RBG_negative(IVC* srtdst);
int VC_rgb_to_gray(IVC* src, IVC* dst);

//Aula que faltei
int vc_rgb_to_hsv(IVC* src, IVC* dst);


int vc_hsv_segmentation(IVC* src, IVC* dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax);
int vc_scale_gray_to_rgb(IVC* src, IVC* dst);


int vc_count_pixeis(IVC* src);


//22-03-2024
int vc_gray_to_binary(IVC* src, IVC* dst, int threshold);
int vc_gray_to_binary_global_mean(IVC* src, IVC* dst);
int vc_gray_to_binary_midpoint(IVC* src, IVC* dst, int kernel);
int vc_gray_to_binary_bernsen(IVC* src, IVC* dst, int kernel, int cMin);
int vc_binary_dilate(IVC* src, IVC* dst, int kernel);
int vc_binary_erode(IVC* src, IVC* dst, int kernel);
int vc_difference(IVC* src1, IVC* src2, IVC* dst);
int vc_inicialize(IVC* src1, IVC * src2, IVC* dst);
int vc_binary_open(IVC* src, IVC* dst, int kernel_dil, int kernel_ero);
int vc_binary_close(IVC* src, IVC* dst, int kernel_dil, int kernel_ero);
int vc_gray_histogram_show(IVC* src, IVC* dst);



//aula 10-05-2024
int vc_gray_edge_prewitt(IVC* src, IVC* dst, float th);
int vc_gray_edge_sobel(IVC* src, IVC* dst, float th);


//aula 13-05-2024
int vc_gray_lowpass_mean_filter(IVC* src, IVC* dst, int kernelsize);
void bubbleSort(int* array);


//trabalho Pratico
int vc_rgb_to_hsv_OCV(IVC* src, IVC* dst);


int findColorResistor(IVC* src, OVC* etiquetas, int position, cores* resistor);
int resistorValues(OVC* etiquetas, int position, cores* resistor);
int funcaoPrincipal(IVC* img);
int drawRectangle(IVC* srcdst, int xtl, int ytl, int xbr, int ybr, int r, int g, int b);