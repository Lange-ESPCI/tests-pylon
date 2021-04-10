#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include "functions.h"

/* Simple error handling. */
#define CHECK( errc ) if ( GENAPI_E_OK != errc ) printErrorAndExit( errc )

int main(void) {
	
	GENAPIC_RESULT res; // Type du retour des méthodes Pylon
	size_t numDevices; // Nombre de caméra disponibles
	PYLON_DEVICE_HANDLE hDev; // Handle référant à la caméra
	_Bool isAvail; // Retour des fonctions de disponibilité
	size_t payloadSize = 0; // Taille d'une image en octets
	int i; // Compteur
	unsigned char* imgBuf; // Buffer d'acquisition de l'image
	const int numGrabs = 10; // Nombre d'image à récupérer
	clock_t c; // Début du timer
	
	// Initialisation du runtime Pylon
	PylonInitialize();
	
	// On regarde combien de caméras sont disponibles
	res = PylonEnumerateDevices(&numDevices);
	CHECK(res);
	if(numDevices == 0) {
		fprintf(stderr, "Aucune caméra trouvée.\n");
		PylonTerminate();
		pressEnterToExit();
		exit(EXIT_FAILURE);
	} else {
			fprintf(stdout, "Nombre de caméras trouvées : %d\n", numDevices);
	}
	
	// On récupère l'handle de la première caméra trouvée
	res = PylonCreateDeviceByIndex(0, &hDev);
	CHECK(res);
	
	// On ouvre la caméra
	res = PylonDeviceOpen(hDev, PYLONC_ACCESS_MODE_CONTROL | PYLONC_ACCESS_MODE_STREAM);
	CHECK(res);
	
	// Affichage du nom de la caméra
	{
		char buf[256];
		size_t siz = sizeof(buf);
		_Bool isReadable;
		
		isReadable = PylonDeviceFeatureIsReadable(hDev, "DeviceModelName");
		if(isReadable) {
			res = PylonDeviceFeatureToString(hDev, "DeviceModelName", buf, &siz);
			CHECK(res);
			printf("Utilisation de la caméra %s\n", buf);
		}
	}
	
	// Activation du format de pixel Mono8 s'il est disponible
	isAvail = PylonDeviceFeatureIsAvailable(hDev, "EnumEntry_PixelFormat_Mono8");
	if(isAvail) {
		res = PylonDeviceFeatureFromString(hDev, "PixelFormat", "Mono8");
		CHECK(res);
		printf("Format de pixel Mono8 activé.\n");
	}
	
	// Désactive le trigger de début d'acquisition si disponible
	isAvail = PylonDeviceFeatureIsAvailable( hDev, "EnumEntry_TriggerSelector_AcquisitionStart" );
	if (isAvail)
	{
		res = PylonDeviceFeatureFromString( hDev, "TriggerSelector", "AcquisitionStart" );
		CHECK( res );
		res = PylonDeviceFeatureFromString( hDev, "TriggerMode", "Off" );
		CHECK( res );
		printf("Trigger de début d'acquisition desactivé.\n");
	}
	
	// Désactive le trigger de début d'image si disponible
	isAvail = PylonDeviceFeatureIsAvailable( hDev, "EnumEntry_TriggerSelector_FrameStart" );
    if (isAvail)
    {
        res = PylonDeviceFeatureFromString( hDev, "TriggerSelector", "FrameStart" );
        CHECK( res );
        res = PylonDeviceFeatureFromString( hDev, "TriggerMode", "Off" );
        CHECK( res );
        printf("Trigger de début d'image désactivé.\n");
    }
    
    // Détermination de la taille du buffer d'acquisition
    {
		PYLON_STREAMGRABBER_HANDLE hGrabber;
		// Ouverture temporaire d'un flux d'acquisition pour le premier channel
		res = PylonDeviceGetStreamGrabber(hDev, 0, &hGrabber);
		CHECK(res);
		res = PylonStreamGrabberOpen(hGrabber);
		CHECK(res);
		
		res = PylonStreamGrabberGetPayloadSize(hDev, hGrabber, &payloadSize);
		CHECK(res);
		
		res = PylonStreamGrabberClose(hGrabber);
		CHECK(res);
	}
	
	// On alloue la mémoire nécessaire à l'acquisition
	imgBuf = (unsigned char*) malloc(payloadSize);
	if(imgBuf == NULL) { // Toujours vérifier ses mallocs, sinon on peut avoir mallocu
		fprintf(stderr, "Dépassement de la mémoire.\n");
		PylonTerminate();
		pressEnterToExit();
		exit(EXIT_FAILURE);
	}

	// Création du dossier
	mkdir("data", 0777);
	
	// Acquisition d'images dans une boucle
	for(i = 0; i < numGrabs;++i) {
		unsigned char min, max;
		PylonGrabResult_t grabResult;
		_Bool bufferReady;
		
		/*
		 * Acquisition d'une seule image depuis le channel de flux 0
		 * La caméra est en mode d'acquisition image unique
		 * L'acquisition peut prendre jusqu'à 500 ms
		 * */
		 res = PylonDeviceGrabSingleFrame(hDev, 0, imgBuf, payloadSize,
			&grabResult, &bufferReady, 500);
		if(GENAPI_E_OK == res && !bufferReady) {
			printf("Image %d : timeout.\n", i + 1);
		}
		CHECK(res);
		
		if(grabResult.Status == Grabbed) {
			// Succès. Processing de l'image
			getMinMax(imgBuf, grabResult.SizeX, grabResult.SizeY, &min, &max);
			printf("Acquisition de l'image #%2d. Valeur grise min = %3u, max = %3u\n",
				i + 1, min, max);
			
			char path[11];
			sprintf(path, "data/%d.png", i + 1);
			c = clock();
			saveImgToPng(imgBuf, path, grabResult.SizeX, grabResult.SizeY);
			c = clock() - c;
			double time_taken = ((double) c)/CLOCKS_PER_SEC;
			printf("Temps écoulé : %fs\n", time_taken);
		} else if(grabResult.Status == Failed){
			fprintf(stderr, "L'image %d n'as pas pu être acquise avec succès. Code d'erreur : 0x%08X/\n",
				i + 1, grabResult.ErrorCode);
		}
	}
	
	// Libération des ressources de la caméra
	res = PylonDeviceClose(hDev);
	CHECK(res);
	res = PylonDestroyDevice(hDev);
	CHECK(res);
	
	// On oublie pas de free les mallocs, pour ne toujours pas avoir mallocu
	free(imgBuf);
	
	pressEnterToExit();
	
	//Libération du runtime Pylon
	PylonTerminate();
	
	return EXIT_SUCCESS;
}


