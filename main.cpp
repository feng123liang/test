
#include <opencv2/opencv.hpp>

#include <filesystem>
#include <iostream>
#include <cstring>
// camera open
#include "cameraOpen/include/MvCameraControl.h"

// camera detection
#include "detector.hpp"


const int bin_tres = 130;
const int detect_color = 1;
// 分别为二值化阈值与Detector要检测的颜色

Detector detector_(bin_tres, detect_color);
 // 0 for picture testing, 1 for video testing, 2 for realtime camera testing 
int Mode;

// 从这里到代码第228行 are camera reading relevant code, no need to inspect them in this stage
void PressEnterToExit(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
    fprintf(stderr, "\nPress enter to exit.\n");
    while (getchar() != '\n')
        ;
}
bool PrintDeviceInfo(MV_CC_DEVICE_INFO *pstMVDevInfo)
{
    if (NULL == pstMVDevInfo)
    {
        printf("The Pointer of pstMVDevInfo is NULL!\n");
        return false;
    }
    if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE)
    {
        int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
        int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
        int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
        int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);
        // ch:打印当前相机ip和用户自定义名字 | en:print current ip and user defined name
        printf("Device Model Name: %s\n", pstMVDevInfo->SpecialInfo.stGigEInfo.chModelName);
        printf("CurrentIp: %d.%d.%d.%d\n", nIp1, nIp2, nIp3, nIp4);
        printf("UserDefinedName: %s\n\n", pstMVDevInfo->SpecialInfo.stGigEInfo.chUserDefinedName);
    }
    else if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE)
    {
        printf("Device Model Name: %s\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chModelName);
        printf("UserDefinedName: %s\n\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName);
    }
    else
    {
        printf("Not support.\n");
    }
    return true;
}
static void __stdcall ImageCallBackEx(unsigned char *pData, MV_FRAME_OUT_INFO_EX *pFrameInfo, void *pUser)
{
    if (pFrameInfo)
    {
        printf("GetOneFrame, Width[%d], Height[%d], nFrameNum[%d]\n",
               pFrameInfo->nWidth, pFrameInfo->nHeight, pFrameInfo->nFrameNum);

        cv::Mat mat(pFrameInfo->nHeight, pFrameInfo->nWidth, CV_8UC1, pData);
        cv::Mat imageRGB;
        cv::cvtColor(mat, imageRGB, cv::COLOR_BayerRG2RGB);

        cv::Mat clonedFrame = imageRGB.clone();
        auto timestamp = std::chrono::system_clock::now();


        cv::imshow("originalframe", clonedFrame);
        detector_.WholeProcess(clonedFrame);
        cv::imshow("Result", clonedFrame);

        cv::waitKey(1);

    }
}
void videoGet()
{

    int nRet = MV_OK;
    void *handle = NULL;
    do
    {
        // ch:初始化SDK | en:Initialize SDK
        nRet = MV_CC_Initialize();
        if (MV_OK != nRet)
        {
            printf("Initialize SDK fail! nRet [0x%x]\n", nRet);
            break;
        }
        MV_CC_DEVICE_INFO_LIST stDeviceList;
        memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
        // 枚举设备
        // enum device
        nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &stDeviceList);
        if (MV_OK != nRet)
        {
            printf("MV_CC_EnumDevices fail! nRet [%x]\n", nRet);
            break;
        }
        if (stDeviceList.nDeviceNum > 0)
        {
            for (int i = 0; i < stDeviceList.nDeviceNum; i++)
            {
                printf("[device %d]:\n", i);
                MV_CC_DEVICE_INFO *pDeviceInfo = stDeviceList.pDeviceInfo[i];
                if (NULL == pDeviceInfo)
                {
                    break;
                }
                PrintDeviceInfo(pDeviceInfo);
            }
        }
        else
        {
            printf("Find No Devices!\n");
            break;
        }
        printf("Please Intput camera index: ");
        unsigned int nIndex = 0;

        if (nIndex >= stDeviceList.nDeviceNum)
        {
            printf("Intput error!\n");
            break;
        }
        // 选择设备并创建句柄
        // select device and create handle
        nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]);
        if (MV_OK != nRet)
        {
            printf("MV_CC_CreateHandle fail! nRet [%x]\n", nRet);
            break;
        }
        // 打开设备
        // open device
        nRet = MV_CC_OpenDevice(handle);
        if (MV_OK != nRet)
        {
            printf("MV_CC_OpenDevice fail! nRet [%x]\n", nRet);
            break;
        }

        // ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera)
        if (stDeviceList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE)
        {
            int nPacketSize = MV_CC_GetOptimalPacketSize(handle);
            if (nPacketSize > 0)
            {
                nRet = MV_CC_SetIntValue(handle, "GevSCPSPacketSize", nPacketSize);
                if (nRet != MV_OK)
                {
                    printf("Warning: Set Packet Size fail nRet [0x%x]!\n", nRet);
                }
            }
            else
            {
                printf("Warning: Get Packet Size fail nRet [0x%x]!\n", nPacketSize);
            }
        }

        // 设置触发模式为off
        // set trigger mode as off
        nRet = MV_CC_SetEnumValue(handle, "TriggerMode", 0);
        if (MV_OK != nRet)
        {
            printf("MV_CC_SetTriggerMode fail! nRet [%x]\n", nRet);
            break;
        }
        // 注册抓图回调
        // register image callback
        nRet = MV_CC_RegisterImageCallBackEx(handle, ImageCallBackEx, handle);
        if (MV_OK != nRet)
        {
            printf("MV_CC_RegisterImageCallBackEx fail! nRet [%x]\n", nRet);
            break;
        }
        // 开始取流
        // start grab image
        nRet = MV_CC_StartGrabbing(handle);
        if (MV_OK != nRet)
        {
            printf("MV_CC_StartGrabbing fail! nRet [%x]\n", nRet);
            break;
        }
        PressEnterToExit();
        // 停止取流
        // end grab image
        nRet = MV_CC_StopGrabbing(handle);
        if (MV_OK != nRet)
        {
            printf("MV_CC_StopGrabbing fail! nRet [%x]\n", nRet);
            break;
        }
        // 关闭设备
        // close device
        nRet = MV_CC_CloseDevice(handle);
        if (MV_OK != nRet)
        {
            printf("MV_CC_CloseDevice fail! nRet [%x]\n", nRet);
            break;
        }
        // 销毁句柄
        // destroy handle
        nRet = MV_CC_DestroyHandle(handle);
        if (MV_OK != nRet)
        {
            printf("MV_CC_DestroyHandle fail! nRet [%x]\n", nRet);
            break;
        }
        handle = NULL;
    } while (0);
    if (handle != NULL)
    {
        MV_CC_DestroyHandle(handle);
        handle = NULL;
    }
    // ch:反初始化SDK | en:Finalize SDK
    MV_CC_Finalize();
    printf("exit\n");
}


int main() {
  // std::cout << "current file_path is: " << __FILE__ << std::endl;

    std::cout << "Choose your mode to test your algorithm: enter 0 for testing "
               "pictures in the pic folder, enter 1 to test one video in the "
               "video folder, enter 2 to open the camera for real time input: \n";
    int inputMode = 0;
    std::cin >> inputMode;
    Mode = inputMode;
    if (Mode == 0) {
    // Specify the folder containing the images to process
    std::string folderPath = "../pic"; //图片文件夹路径
    std::string savePath = "../output";
    cv::Mat ori_image;
    std::string picName;
    for (const auto &entry : std::filesystem::directory_iterator(folderPath)) { //遍历目标文件夹下的所有图片
      if (entry.is_regular_file()) {
        picName = entry.path().string();

        std::cout << "processing " << picName << std::endl;
        std::cout
            << "press esc to quit, press other key to process next image. \n";

        ori_image = cv::imread(picName);

        if (ori_image.empty()) {
          std::cerr << "Failed to read image: " << picName << std::endl;
          continue;
        }

        cv::cvtColor(ori_image, ori_image, cv::COLOR_BGR2RGB); // 由于cv::imread得到的图像的默认色彩通道是BGR，需要转化成RBG

        detector_.WholeProcess(ori_image);

        cv::cvtColor(ori_image, ori_image, cv::COLOR_RGB2BGR); // 由于转化回BGR以正常 imshow 显示
        cv::imshow("Result", ori_image);

        

        std::string temsavePath = savePath;
        std::string tems  = "";
        for (int i=picName.length()-1;i>=0;i--){
            tems += picName[i];
            if(picName[i] == '/') break;
        }
        for (int i=tems.length() - 1;i>=0;i--){
            temsavePath += tems[i];
        }
        bool result = cv::imwrite(temsavePath, ori_image);

        if (result) {
            std::cout << "Image saved successfully as " << temsavePath << std::endl;
        } else {
            std::cerr << "Error saving image!" << std::endl;
            return -1;
        }
        int key = cv::waitKey(0);
        if (key == 27) { // If ESC key is pressed, exit the loop
          break;
        }
      }
    }
    std::cout << "finish testing all the images in the folder\n";
  }

  else if (Mode == 1) {
        cv::VideoCapture cap("../video/test.mp4"); // 视频文件读取路径

    if (!cap.isOpened()) {
        std::cerr << "Error opening video file!" << std::endl;
        return -1;
    }

    cv::Mat frame;
    cv::Mat ori_image;
    while (cap.read(frame)) {
        // Process each frame here
        
        // Example: Convert frame to grayscale
        
        cv::cvtColor(frame, ori_image, cv::COLOR_BGR2RGB);

        // Display the processed frame
        detector_.WholeProcess(ori_image);
        cv::cvtColor(ori_image, ori_image, cv::COLOR_RGB2BGR);

            // Text to be added
            // std::string text1 = "First line";
            // std::string text2 = "Second line";

            // // Coordinates for the bottom-left corner of the text
            // int x = image.cols - 300; // Adjust the x position as needed
            // int y = image.rows - 20;  // Adjust the y position as needed

            // // Font settings
            // int fontFace = cv::FONT_HERSHEY_SIMPLEX;
            // double fontScale = 1.0;
            // cv::Scalar fontColor(255, 255, 255);  // Text color in BGR format
            // int thickness = 2;

            // // Add text to the image
            // cv::putText(image, text1, cv::Point(x, y), fontFace, fontScale, fontColor, thickness);
            // cv::putText(image, text2, cv::Point(x, y + 30), fontFace, fontScale, fontColor, thickness);


        cv::imshow("Result", ori_image);
        
        // Wait for 30ms and check if the user pressed 'q' to exit
        if (cv::waitKey(30) == 'q') {
            break;
        }
    }

    cap.release();
    cv::destroyAllWindows();
    std::cout << "video ended\n";
  }
  else if( Mode == 2){
    videoGet(); //实时处理摄像头流
  }
  else{
    std::cout << "invalid order\n";
  }
  return 0;
}
