#include <iostream>
#include <string>
#include <chrono>
#include <opencv2\opencv.hpp>
#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\videoio.hpp>

extern "C" {
#include "vc.h"
using namespace cv;
}


void vc_timer(void) {
	static bool running = false;
	static std::chrono::steady_clock::time_point previousTime = std::chrono::steady_clock::now();

	if (!running) {
		running = true;
	}
	else {
		std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
		std::chrono::steady_clock::duration elapsedTime = currentTime - previousTime;

		// Tempo em segundos.
		std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(elapsedTime);
		double nseconds = time_span.count();

		std::cout << "Tempo decorrido: " << nseconds << "segundos" << std::endl;
		std::cout << "Pressione qualquer tecla para continuar...\n";
		std::cin.get();
	}
}

int main(void) {
	// V�deo
	char videofile[20] = "video_resistors.mp4";
	cv::VideoCapture capture;
	struct
	{
		int width, height;
		int ntotalframes;
		int fps;
		int nframe;
	} video;
	// Outros
	std::string str;
	int key = 0;

	/* Leitura de v�deo de um ficheiro */
	/* NOTA IMPORTANTE:
	O ficheiro video.avi dever� estar localizado no mesmo direct�rio que o ficheiro de c�digo fonte.
	*/
	capture.open(videofile);

	/* Em alternativa, abrir captura de v�deo pela Webcam #0 */
	//capture.open(0, cv::CAP_DSHOW); // Pode-se utilizar apenas capture.open(0);

	/* Verifica se foi poss�vel abrir o ficheiro de v�deo */
	if (!capture.isOpened())
	{
		std::cerr << "Erro ao abrir o ficheiro de v�deo!\n";
		return 1;
	}

	/* N�mero total de frames no v�deo */
	video.ntotalframes = (int)capture.get(cv::CAP_PROP_FRAME_COUNT);
	/* Frame rate do v�deo */
	video.fps = (int)capture.get(cv::CAP_PROP_FPS);
	/* Resolu��o do v�deo */
	video.width = (int)capture.get(cv::CAP_PROP_FRAME_WIDTH);
	video.height = (int)capture.get(cv::CAP_PROP_FRAME_HEIGHT);

	/* Cria uma janela para exibir o v�deo */
	cv::namedWindow("VC - VIDEO", cv::WINDOW_AUTOSIZE);

	/* Inicia o timer */
	vc_timer();

	//Variaveis alteradas
	int i, j, res = 0, resColor = 0, k;
	IVC* image = vc_image_new(video.width, video.height, 3, 255);
	IVC* hsv = vc_image_new(video.width, video.height, 3, 255);
	IVC* segmentation = vc_image_new(video.width, video.height, 1, 255);
	IVC* closefilter = vc_image_new(video.width, video.height, 1, 255);
	IVC* blob1 = vc_image_new(video.width, video.height, 1, 255);

	cores* resistor;
	resistor = (cores*)malloc(3 * sizeof(cores));

	for (i = 0; i < 3; i++) {
		resistor[i].color[0] = -1;
		resistor[i].color[1] = -1;
		resistor[i].color[2] = -1;
		resistor[i].x[0] = -1;
		resistor[i].x[1] = -1;
		resistor[i].x[2] = -1;
		resistor[i].y[0] = -1;
		resistor[i].y[1] = -1;
		resistor[i].y[2] = -1;
		resistor[i].height = -1;
		resistor[i].width = -1;
		resistor[i].xl= -1;
		resistor[i].yl= -1;
	}

	cv::Mat frame;
	cv::Mat frame1 = cv::Mat::zeros(video.height, video.width, CV_8UC1);;
	Mat kernel;
	//Variaveis alteradas

	while (key != 'q') {
		/* Leitura de uma frame do v�deo */
		capture.read(frame);

		/* Verifica se conseguiu ler a frame */
		if (frame.empty()) break;

		/* N�mero da frame a processar */
		video.nframe = (int)capture.get(cv::CAP_PROP_POS_FRAMES);

		//----------------------Codigo alterado----------------------

		// Copia dados de imagem da estrutura cv::Mat para uma estrutura IVC
		memcpy(image->data, frame.data, video.width * video.height * 3);

		//Alterar BGR para HSV
		vc_rgb_to_hsv_OCV(image, hsv);

		//Segmentar a imagem (selecionar apenas o fundo)
		vc_hsv_segmentation(hsv, segmentation, valores[resistencia][hMin], valores[resistencia][hMax], valores[resistencia][sMin], valores[resistencia][sMax], valores[resistencia][vMin], valores[resistencia][vMax]);

		//Inverter a imagem de forma a que fique tudo selecionado menos o fundo
		VC_gray_negative(segmentation);


		memcpy(frame1.data, segmentation->data, video.width * video.height);


		//----Fecho----//
		kernel = getStructuringElement(MORPH_RECT, Size(9, 9));
		Mat dilated;
		dilate(frame1, dilated, kernel);
		// Aplicar a operação de erosão
		kernel = getStructuringElement(MORPH_RECT, Size(7, 7));
		Mat eroded;
		erode(dilated, eroded, kernel);
		//----Fecho----//


		//----Abertura----//
		/// Aplicar a operação de erosão
		kernel = getStructuringElement(MORPH_RECT, Size(9, 9));
		erode(eroded, dilated, kernel);

		kernel = getStructuringElement(MORPH_RECT, Size(11, 11));
		// Aplicar a operação de dilatação
		dilate(dilated, eroded, kernel);
		//----Abertura----//

		memcpy(closefilter->data, eroded.data, image->width * image->height);


		//etiquetagem
		int nblobs;
		OVC* blobs;
		blobs = vc_binary_blob_labelling(closefilter, blob1, &nblobs);
		vc_binary_blob_info(blob1, blobs, nblobs);
		
		int resNumber = 0;

		if (blobs != NULL) {
			for (i = 0; i < nblobs; i++) {
				int wtol = 35, htol = 20, width = 170, height = 65;			//valores de referencia para resistencias
				if ((blobs[i].width > (width - wtol + 10)) && (blobs[i].width < (width + wtol + 20)) && (blobs[i].height > (height - htol)) && (blobs[i].height < (height + htol)) && blobs[i].y >= 5) {
					resistor[resNumber].xl = blobs[i].x;
					resistor[resNumber].yl = blobs[i].y;
					resistor[resNumber].width = blobs[i].width;
					resistor[resNumber].height = blobs[i].height;
					resistor[resNumber].x[0] = -1;
					resistor[resNumber].x[1] = -1;
					resistor[resNumber].x[2] = -1;
					resNumber++;
				}
			}
		}
		free(blobs);
		int color;
		for (color = 0; color < 6; color++) {
			//Segmenta�ao da imagem hsv
			vc_hsv_segmentation(hsv, segmentation, valores[color][hMin], valores[color][hMax], valores[color][sMin], valores[color][sMax], valores[color][vMin], valores[color][vMax]);

			memcpy(frame1.data, segmentation->data, video.width * video.height);



			kernel = getStructuringElement(MORPH_RECT, Size(5, 5));
			Mat dilated;
			dilate(frame1, dilated, kernel);

			Mat eroded;
			erode(dilated, eroded, kernel);

			erode(eroded, dilated, kernel);
			kernel = getStructuringElement(MORPH_RECT, Size(13, 13));
			dilate(dilated, eroded, kernel);


			memcpy(closefilter->data, eroded.data, image->width * image->height);


			//etiquetagem
			blobs = vc_binary_blob_labelling(closefilter, blob1, &nblobs);
			vc_binary_blob_info(blob1, blobs, nblobs);

			if (blobs != NULL) {
				for (i = 0; i < nblobs; i++) {
					for (j = 0; j < resNumber; j++) {
						//calculo da area da resistencia
						// x    xd
						// y    yd
						int xd, yd, minArea = 240;
						xd = resistor[j].xl + resistor[j].width;
						yd = resistor[j].yl + resistor[j].height;
						if ((blobs[i].xc > resistor[j].xl) && (blobs[i].xc < xd) && (blobs[i].yc > resistor[j].yl) && (blobs[i].yc < yd) && blobs[i].area > minArea ) {
							if (resistor[j].x[2] == -1) {
								int pos = 0;
								while (resistor[j].x[pos] != -1)
									pos++;
								if (pos < 3) {
									resistor[j].color[pos] = color;
									resistor[j].x[pos] = blobs[i].xc;
									resistor[j].y[pos] = blobs[i].yc;
								}
							}
						}
					}
				}
			}

		}

		for (int i = 0; i < resNumber; i++) {

			//ordenar as cores de cada resistencia
			int x = 0, y = 0, aux = 0;

			//organizar as cores em ordem a x
			for (int t = 0; t < 2; t++) {
				for (j = 1; j < 3; j++) {
					if (resistor[i].x[t] > resistor[i].x[j]) {
						x = resistor[i].x[t];
						y = resistor[i].y[t];
						aux = resistor[i].color[t];
						resistor[i].x[t] = resistor[i].x[j];
						resistor[i].y[t] = resistor[i].y[j];
						resistor[i].color[t] = resistor[i].color[j];
						resistor[i].x[j] = x;
						resistor[i].y[j] = y;
						resistor[i].color[j] = aux;
					}
				}
			}

			//voltar a por o valor da resistencia a 0
			resistor[i].value = 0;
			//calculo dos valores das cores
			for (int t = 0; t < 3; t++) {
				if (t == 2) {
					resistor[i].value *= table[resistor[i].color[t]][t];
				}
				if (t == 1) {
					resistor[i].value += table[resistor[i].color[t]][t];
				}if (t == 0) {
					resistor[i].value += table[resistor[i].color[t]][t] * 10;
				}
			}

			// Desenhar o quadrado
			memcpy(image->data, frame.data, image->width * image->height * image->channels);
			drawRectangle(image, resistor[i].xl, resistor[i].yl, resistor[i].xl + resistor[i].width, resistor[i].yl + resistor[i].height, 0, 255, 0);
			memcpy(frame.data, image->data, image->width* image->height * image->channels);
			str = std::string("OHMS: ").append(std::to_string(resistor[i].value));
			cv::putText(frame, str, cv::Point(resistor[i].xl + resistor[i].width, resistor[i].yl + resistor[i].height), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		}


		free(blobs);

		// +++++++++++++++++++++++++

		/* Exibe a frame */
		cv::imshow("VC - VIDEO", frame);

		/* Sai da aplica��o, se o utilizador premir a tecla 'q' */
		key = cv::waitKey(1);
	}

	/* Para o timer e exibe o tempo decorrido */
	vc_timer();

	/* Fecha a janela */
	cv::destroyWindow("VC - VIDEO");

	/* Fecha o ficheiro de v�deo */
	capture.release();


	//libertar memoria 
	vc_image_free(image);
	vc_image_free(hsv);
	vc_image_free(segmentation);
	vc_image_free(closefilter);
	vc_image_free(blob1);
	free(resistor);
	//libertar memoria 

	return 0;
}