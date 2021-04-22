#include <chrono>
#include <sys/stat.h>
#include <sys/types.h>

#include <pylon/PylonIncludes.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/video.hpp>

using namespace Pylon;
using namespace cv;
using namespace std;
using namespace std::chrono;

// Le nombre d'images max à récupérer
static const uint32_t c_countOfImagesToGrab = 1000;
// Le nombre maximum d'octets écrits avant d'arrêter l'acquisition
static const int64_t c_maxImageDataBytesTreshold = 50 * 1024 * 1024;

int main(int argc, char* argv[]) {

    int exitCode = 0;

    PylonInitialize();

    try {
        // Crée un objet avec la première caméra trouvée
        CInstantCamera camera(CTlFactory::GetInstance().CreateFirstDevice());

        cout << "Utilisation de l'appareil " << camera.GetDeviceInfo().GetVendorName() << " "
            << camera.GetDeviceInfo().GetModelName() << endl;

        // Ouverture de la caméra
        camera.Open();

        // Récupération des caractéristiques de la caméra
        CIntegerParameter width(camera.GetNodeMap(), "Width");
        CIntegerParameter height(camera.GetNodeMap(), "Height");
        CEnumParameter pixelFormat(camera.GetNodeMap(), "PixelFormat");

        // Choix de la résolution d'acquisition
        width.TrySetValue(640, IntegerValueCorrection_Nearest);
        height.TrySetValue(480, IntegerValueCorrection_Nearest);

        CPixelTypeMapper pixelTypeMapper(&pixelFormat);
        EPixelType pixelType = pixelTypeMapper.GetPylonPixelTypeFromNodeValue(pixelFormat.GetIntValue());

        CImageFormatConverter formatConverter;
        formatConverter.OutputPixelFormat = PixelType_BGR8packed;
        CPylonImage imagePylon;
        int grabbedImages = 0;

        VideoWriter cvVideoCreator;
        Mat imageCV;

        mkdir("data", 0777);
        string videoFileName = "data/output.avi";

        Size frameSize = Size((int) width.GetValue(), (int) height.GetValue());
        int codec = VideoWriter::fourcc('R', 'G', 'B', 'A');

        cvVideoCreator.open(videoFileName, codec, 25, frameSize, true);

        camera.StartGrabbing(c_countOfImagesToGrab, GrabStrategy_LatestImageOnly);

        CGrabResultPtr ptrGrabResult;

        auto t1 = high_resolution_clock::now();
        auto t2 = high_resolution_clock::now();
        float duration;

        auto tStart = high_resolution_clock::now();

        while(camera.IsGrabbing()) {
            camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);

            if(ptrGrabResult->GrabSucceeded()) {

                formatConverter.Convert(imagePylon, ptrGrabResult);
                imageCV = Mat(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_8UC3, (uint8_t *) imagePylon.GetBuffer());

                cvVideoCreator.write(imageCV);

                t2 = high_resolution_clock::now();
                duration = 1e3 / duration_cast<milliseconds>(t2 - t1).count();
                cout << "FPS : " << duration << endl;
                t1 = t2;
            }
        }

        auto tStop = high_resolution_clock::now();
        duration = duration_cast<milliseconds>(tStop - tStart).count();
        cout << "Durée d'acquisition : " << duration << "ms" << endl;

        camera.Close();
    } catch(const GenericException& e) {
        cerr << "Une erreur s'est produite." << endl
            << e.GetDescription() << endl;
        exitCode = 1;
    }

    PylonTerminate();

    return exitCode;
}