#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include "functions.h"

/* Simple error handling. */
#define CHECK( errc ) if ( GENAPI_E_OK != errc ) printErrorAndExit( errc )

#define NUM_BUFFERS 5 // Nombre de buffers utilisés pour capturer les images
#define NUM_GRABS 100 // Nombre d'images enregistrées

int main(void) {
	
	GENAPIC_RESULT res; // Type du retour des méthodes Pylon
	size_t numDevices; // Nombre de caméra disponibles
	PYLON_DEVICE_HANDLE hDev; // Handle référant à la caméra
    PYLON_STREAMGRABBER_HANDLE hGrabber; // Handle du  flux d'acquisition
    PYLON_WAITOBJECTS_HANDLE hWait; // Handle d'attente du buffer d'acquisition
	size_t payloadSize = 0; // Taille d'une image en octets
    unsigned char* buffers[NUM_BUFFERS]; // Buffer utilisé pour l'acquisition
    PYLON_STREAMBUFFER_HANDLE bufHandles[NUM_BUFFERS];
    _Bool isAvail; // Retour des fonctions de disponibilité
    _Bool isReady; // Retour de fonctions
    PylonGrabResult_t grabResult; //Stockage du résultat de l'acquisition
    int nGrabs; // Compte le nombre de buffers acquis
    size_t nStreams; // Nombre de flux supporté par l'appareil
	int i; // Compteur
	clock_t c; // Début du timer
	FILE *fp; // Fichier d'image
	
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
	
	// Activation du format de pixel RGB8 s'il est disponible
	isAvail = PylonDeviceFeatureIsAvailable(hDev, "EnumEntry_PixelFormat_RGB8");
	if(isAvail) {
		res = PylonDeviceFeatureFromString(hDev, "PixelFormat", "RGB8");
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


    //Désactive le trigger du début de burst d'image
    isAvail = PylonDeviceFeatureIsAvailable( hDev, "EnumEntry_TriggerSelector_FrameBurstStart" );
    if (isAvail)
    {
        res = PylonDeviceFeatureFromString( hDev, "TriggerSelector", "FrameBurstStart" );
        CHECK( res );
        res = PylonDeviceFeatureFromString( hDev, "TriggerMode", "Off" );
        CHECK( res );
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

    // Utilisation du mode continu d'acquisition d'images
    res = PylonDeviceFeatureFromString(hDev, "AcquisitionMode", "Continuous");
    CHECK(res);
    
    // Obtention du nombre de flux supporté par la caméra et la couche de transport
    res = PylonDeviceGetNumStreamGrabberChannels(hDev, &nStreams);
    CHECK(res);
    if(nStreams < 1) {
        fprintf(stderr, "La couche de transport ne supporte les flux d'images\n");
        PylonTerminate();
        pressEnterToExit();
        exit(EXIT_FAILURE);
    }

    // Ouverture d'un flux d'acquisition pour le premier canal
    res = PylonDeviceGetStreamGrabber(hDev, 0, &hGrabber);
    CHECK(res);
    res = PylonStreamGrabberOpen(hGrabber);
    CHECK(res);

    // Récupération d'un handle de l'attente du flux d'acquisition
    res = PylonStreamGrabberGetWaitObject(hGrabber, &hWait);
    CHECK(res);

    // Détermination de la taille minimale du du buffer d'acquisition
    res = PylonStreamGrabberGetPayloadSize(hDev, hGrabber, &payloadSize);
    CHECK(res);

    // Allocation de la mémoire pour l'acquisition
    for(i = 0; i < NUM_BUFFERS; ++i) {
        buffers[i] = (unsigned char*) malloc(payloadSize);
        if(NULL == buffers[i]) {
            fprintf(stderr, "Plus de mémoire disponible !\n");
            PylonTerminate();
            pressEnterToExit();
            exit(EXIT_FAILURE);
        }
    }

    // Annonce du nombre et de la taille des buffers au flux d'acquisition
    res = PylonStreamGrabberSetMaxNumBuffer(hGrabber, NUM_BUFFERS);
    CHECK(res);
    res = PylonStreamGrabberSetMaxBufferSize(hGrabber, payloadSize);
    CHECK(res);

    // Allocation des ressources pour l'acquisition
    res = PylonStreamGrabberPrepareGrab(hGrabber);
    CHECK(res);

    // Enregistrement des buffers auprès du flux d'acquisition
    for(i = 0; i < NUM_BUFFERS; ++i) {
        res = PylonStreamGrabberRegisterBuffer(hGrabber, buffers[i], payloadSize, &bufHandles[i]);
        CHECK(res);
    }

    // Mets les buffer dans la file d'attente d'entrée du flux d'acquisition
    for(i = 0; i < NUM_BUFFERS; ++i){
        res = PylonStreamGrabberQueueBuffer(hGrabber, bufHandles[i], (void *) i);
        CHECK(res);
    }

    // Démarre le moteur d'acquisition d'images
    res = PylonStreamGrabberStartStreamingIfMandatory(hGrabber);
    CHECK(res);

    // Autorise la caméra à acquérir des images
    res = PylonDeviceExecuteCommandFeature(hDev, "AcquisitionStart");
    CHECK(res);

	// Création du dossier
	mkdir("data", 0777);
	
	// Acquisition d'images dans une boucle
    nGrabs = 0;
    c = clock();
	while(nGrabs < NUM_GRABS) {
		unsigned char min, max;
        size_t bufferIndex;
        // Attends jusqu'à 1s qu'un buffer soit rempli
        res = PylonWaitObjectWait(hWait, 1000, &isReady);
        CHECK(res);
        if(!isReady) {
            fprintf(stderr, "Timeout pour l'acquisition d'image\n");
            break;
        }

        res = PylonStreamGrabberRetrieveResult(hGrabber, &grabResult, &isReady);
        CHECK(res);
        if(!isReady) {
            fprintf(stderr, "Échec de la récupération d'un résultat d'acquisition\n");
            break;
        }

        nGrabs++;

        //On obtient l'indice du buffer grâce au contexte
        bufferIndex = (size_t) grabResult.Context;

        // On vérifie que l'image a été récupérée avec succès
        if(grabResult.Status == Grabbed) {
            // Succès. On traite l'image
            unsigned char* buffer; 
            buffer = (unsigned char*) grabResult.pBuffer;

            getMinMax(buffer, grabResult.SizeX, grabResult.SizeY, &min, &max);
            printf( "Grabbed frame %2d into buffer %2d. Min. gray value = %3u, Max. gray value = %3u\n",
                    nGrabs, (int) bufferIndex, min, max );
			c = clock() - c;
			double time_taken = ((double) c)/CLOCKS_PER_SEC;
			printf("Temps d'acquisition : %fs\n", time_taken);
        } else if(grabResult.Status == Failed) {
            fprintf(stderr, "Erreur durant l'acquisition de l'image %d. Code d'erreur : 0x%08X\n",
                nGrabs, grabResult.ErrorCode);
        }

        res = PylonStreamGrabberQueueBuffer(hGrabber, grabResult.hBuffer, (void *) bufferIndex);
        CHECK(res);
	
	}

    // Arrêt de la caméra
    res = PylonDeviceExecuteCommandFeature(hDev, "AcquisitionStop");
    CHECK(res);


    // Arrêt du moteur d'acquisition d'images
    res = PylonStreamGrabberStopStreamingIfMandatory(hGrabber);

    // On s'assure que tous les buffer en attente soit poussés vers la sortie
    res = PylonStreamGrabberFlushBuffersToOutput(hGrabber);
    CHECK(res);
    do {
        res = PylonStreamGrabberRetrieveResult(hGrabber, &grabResult, &isReady);
        CHECK(res);
    } while(isReady);

    // Libération des buffers
    for(i = 0; i < NUM_BUFFERS; ++i) {
        res = PylonStreamGrabberDeregisterBuffer(hGrabber, bufHandles[i]);
        CHECK(res);
        free(buffers[i]);
    }

    // Libération des ressources d'acquisition
    res = PylonStreamGrabberFinishGrab(hGrabber);
    CHECK(res);

    // Fermeture du flux d'acquisition
    res = PylonStreamGrabberClose(hGrabber);
    CHECK(res);

	
	// Libération des ressources de la caméra
	res = PylonDeviceClose(hDev);
	CHECK(res);
	res = PylonDestroyDevice(hDev);
	CHECK(res);
	
	pressEnterToExit();
	
	//Libération du runtime Pylon
	PylonTerminate();
	
	return EXIT_SUCCESS;
}


