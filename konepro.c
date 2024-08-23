#include<stdlib.h>
#include<stdint.h>
#include<stdio.h>
#include<string.h>
#include<libusb-1.0/libusb.h>
#include<math.h>

struct profile{
    uint8_t profile1[69];
    uint8_t profile2[69];
    uint8_t profile3[69];
    uint8_t profile4[69];
    uint8_t profile5[69];
    uint8_t dbt; // Debounce Time (This is global and not profile specific)
};

int convertToDataArray(char text[], uint8_t **localdata);
int setDefaultState(libusb_device_handle *handle);
libusb_device_handle* openDevice(void);
int closeDevice(libusb_device_handle* handle);
int getProfileData(libusb_device_handle *handle, int profile, uint8_t *data);
uint8_t getDebounceTime(libusb_device_handle *handle);
void listProfileSettings(struct profile *p, int profile);
int setDebounceTime(libusb_device_handle *handle, uint8_t dbT);

const uint16_t VID = 0x1e7d;
const uint16_t PID = 0x2c88;
int errCheck = 0;


int main(int argc, char *argv[])
{
    
    if(argc < 2 || !strcmp(argv[1],"--help"))
    {
        printf("Usage: konepro OPTIONS\nOPTIONS:\n-l R G B // Left Click Colour (values 0 to 255)\n-r R G B // Right Click Colour (values 0 to 255)\n");
        printf("-lm value // LED Mode (0=Off,1=Fully lit,2=blinking,3=breathing,4=Heartbeat,9=Aimo Intelligent,10=Wave)\n-lb value // LED Brightness (0 to 255)\n");
        printf("-ls value // LED Speed (1 to 11)\n");
        printf("-list profile // List Profile Settings (0 to 4)\n");
        printf("-p value // Polling Rate (0 to 3; 125,250,500,1000)\n");
        printf("-d dpi switch // DPI (minimum: 50, maximum 19,000, increments of 50), switch(0 to 4)(Defaults to 0 if not spcified)\n-ds value // DPI Switcher (0 to 4)\n");
        printf("-prf value // Profile to change (value 0 to 4) Defaults to 0 if omitted\n");
        printf("-default // Factory reset Device\n");
        printf("-dbt value // Sets Debounce Time in milliseconds (value 0 to 10)\n");
        return 0;
    }
    
    int checkProfile = 0;
    for(int i = 0; argv[i] != NULL; i++)
    {
        if(!strcmp(argv[i],"-prf") && i+1 < argc){
            checkProfile = (int)strtol(argv[i+1],NULL,10);
            break;
        }
    }

    libusb_device_handle *devHandle = openDevice();
    if(devHandle == NULL)
    {
        printf("Failed to open device\n");
        return 1;
    }


    struct profile profiles;
    errCheck = getProfileData(devHandle,0,profiles.profile1);
    if(errCheck != 0){
        printf("getProfileData Failed\n");
        goto FailState;
    }
    errCheck = getProfileData(devHandle,1,profiles.profile2);
    if(errCheck != 0){
        printf("getProfileData Failed\n");
        goto FailState;
    }
    errCheck = getProfileData(devHandle,2,profiles.profile3);
    if(errCheck != 0){
        printf("getProfileData Failed\n");
        goto FailState;
    }
    errCheck = getProfileData(devHandle,3,profiles.profile4);
    if(errCheck != 0){
        printf("getProfileData Failed\n");
        goto FailState;
    }
    errCheck = getProfileData(devHandle,4,profiles.profile5);
    if(errCheck != 0){
        printf("getProfileData Failed\n");
        goto FailState;
    }
    profiles.dbt = getDebounceTime(devHandle);
    if(profiles.dbt == 11){
        printf("getDebounceTime Failed\n");
        goto FailState;
    }
    
    uint8_t *currentSettings;
    switch(checkProfile)
    {
        case 0:
            currentSettings = profiles.profile1;
            break;
        case 1:
            currentSettings = profiles.profile2;
            break;
        case 2:
            currentSettings = profiles.profile3;
            break;
        case 3:
            currentSettings = profiles.profile4;
            break;
        case 4:
            currentSettings = profiles.profile5;
            break;
        default:
            printf("Invalid Profile\n");
            closeDevice(devHandle);
            return 1;
    }
    //printf("%d\n",argc);
    //printf("%s\n", argv[0]);
    
    for(int i = 0; argv[i] != NULL; i++)
    {
        if(!strcmp(argv[i],"-l") && i+3 < argc)
        {
            
            currentSettings[38] = (uint8_t)strtol(argv[i+1], NULL, 10); // left click RGB
            currentSettings[39] = (uint8_t)strtol(argv[i+2], NULL, 10);
            currentSettings[40] = (uint8_t)strtol(argv[i+3], NULL, 10);
               
        }
        else if(!strcmp(argv[i],"-r") && i+3 < argc)
        {
            
            currentSettings[43] = (uint8_t)strtol(argv[i+1], NULL, 10); // right click RGB
            currentSettings[44] = (uint8_t)strtol(argv[i+2], NULL, 10);
            currentSettings[45] = (uint8_t)strtol(argv[i+3], NULL, 10);
            
        }
        else if(!strcmp(argv[i],"-p") && i+1 < argc)
        {
            currentSettings[29] = (uint8_t)strtol(argv[i+1], NULL, 10); //Polling rate
            if(currentSettings[29] > 3) goto FailState;
        }
        else if(!strcmp(argv[i], "-lm") && i+1 < argc)
        {
            currentSettings[30] = (uint8_t)strtol(argv[i+1], NULL, 10); //LED Mode
            uint8_t validSettings[] = {0,1,2,3,4,9,10};
            for(int i = 0; i < 7; i++){
                if(currentSettings[30] == validSettings[i]) goto FoundValidSetting;
            }
            goto FailState;
            FoundValidSetting:
                //printf("FoundValidSetting Triggered\n");
                continue;
        }
        else if(!strcmp(argv[i], "-lb") && i+1 < argc)
        {
            currentSettings[32] = (uint8_t)strtol(argv[i+1], NULL, 10); //LED Brightness
        }
        else if(!strcmp(argv[i], "-d") && i+1 < argc)
        {

			int switchMod = 0;
			if(argc > i+2 && argv[i+2][0] <= '4' && argv[i+2][0] >= '0'){ // Decide which switch to change
				unsigned int swtch = (unsigned int)strtol(argv[i+2], NULL, 10);
				if(swtch > 4){
					printf("Invalid DPI switch!\n");
					closeDevice(devHandle);
					return 0;
				}
				switchMod = 2 * swtch; 
			}
            unsigned int dpi = (unsigned int)strtol(argv[i+1], NULL, 10); 
            if(dpi % 50 != 0 || dpi > 19000) goto FailState;
            currentSettings[7+switchMod] = ((dpi/50) % 256);                 // Switch 1 to 4 is +2 elements from the first byte of the previous Switch
            currentSettings[8+switchMod] = (((dpi/50)-currentSettings[7+switchMod]) / 256);
            
        }
        else if(!strcmp(argv[i], "-ds") && i+1 < argc)
        {
            currentSettings[6] = (uint8_t)strtol(argv[i+1], NULL,10); // DPI Switcher
            if(currentSettings[6] > 4) goto FailState;
        }
        else if(!strcmp(argv[i], "-default"))
        {
            errCheck = setDefaultState(devHandle); // Factory Reset
            if(errCheck != 0)
            {
                printf("setDefaultState Failed\n");
                closeDevice(devHandle);
                return 1;
            }
            closeDevice(devHandle);
            return 0;
        }
        else if(!strcmp(argv[i], "-list") && i+1 < argc)
        {
            listProfileSettings(&profiles, (int)strtol(argv[i+1],NULL,10)); // List Profile settings
            closeDevice(devHandle);
            return 0;
        }
        else if (!strcmp(argv[i],"-ls") && i+1 < argc)
        {
            currentSettings[31] = (uint8_t)strtol(argv[i+1],NULL, 10); // LED Speed
            if(currentSettings[31] > 11 || currentSettings[31] == 0) goto FailState;
        }
        else if(!strcmp(argv[i],"-dbt") && i+1 < argc)
        {
            errCheck = setDebounceTime(devHandle,(uint8_t)strtol(argv[i+1],NULL,10));
            if(errCheck != 0){
                printf("setDebounceTime Failed\n");
                closeDevice(devHandle);
                return 1;
            }

            //closeDevice(devHandle);
            //return 0;
        }
        
    }
    int sum = 0;
    for(int i = 0; i < 67; i++)
        sum += currentSettings[i];
    currentSettings[67] = sum % 256; // Checksum
    currentSettings[68] = (sum - currentSettings[67]) / 256;
    
    int errorCheck = libusb_control_transfer(devHandle,0x21,0x09,0x0306,0x0003,currentSettings,0x0045,10000);
    if(errorCheck < 0 ){
        printf("Control transfer failed\n");
        goto FailState;
    }
    
    errCheck = closeDevice(devHandle);
    if(errCheck != 0)
    {
        printf("Close device Failed");
        return 1;
    }
    return 0;

    FailState:
        closeDevice(devHandle);
        printf("FailState Triggered\n");
        return 1;
}

int convertToDataArray(char text[], uint8_t **localdata)
{
    int track = 0;
    char mainTmp[999][5];
    ((*localdata)) = malloc(999 * sizeof(uint8_t));
    if((*localdata) == NULL)
    {
        printf("Failed to allocate memory\n");
        return 1;
    }
    
    for(int byteTrack = 0; text[byteTrack] != '\0'; track++, byteTrack += 2)
    {
        mainTmp[track][0] = '0';
        mainTmp[track][1] = 'x';
        mainTmp[track][2] = text[byteTrack];
        mainTmp[track][3] = text[byteTrack+1];
        mainTmp[track][4] = '\0';
    }
    for(int i = 0; i < track; i++)
    {
        (*localdata)[i] = (uint8_t)strtol(mainTmp[i],NULL,0);
    }
    uint8_t *tmp = realloc((*localdata), track * sizeof(uint8_t));
    if(tmp == NULL)
    {
        printf("Failed to reallocate memory\n");
        free((*localdata));
        return 1;
    }
    (*localdata) = tmp;
    return 0;
}

int setDefaultState(libusb_device_handle *handle)
{
    char defaultPreset[] = "090801000000000000";
    uint8_t *data;
    errCheck = convertToDataArray(defaultPreset,&data);
    if(errCheck != 0)
    {
        printf("convertToDataArray failed\n");
        //closeDevice(handle);
        return 1;
    }

    int errorCheck = libusb_control_transfer(handle,0x21,0x09,0x0309,0x0003,data,0x0009,10000);
    if(errorCheck < 0){
        printf("Control Transfer Failed\n");
        return 1;
    }
    free(data);
    //closeDevice(handle); -- Device is closed after the function is called in the main function.
    return 0;
}

libusb_device_handle* openDevice(void)
{
    errCheck = libusb_init(NULL);
    if(errCheck != 0)
    {
        printf("Init error");
        return NULL;
    }
    
    libusb_device_handle *devHandle = libusb_open_device_with_vid_pid(NULL,VID,PID);
    if(devHandle == NULL)
    {
        printf("Device Handle Error\n");
        return NULL;
    }

    errCheck = libusb_detach_kernel_driver(devHandle,3);
    if(errCheck != 0)
    {
        libusb_attach_kernel_driver(devHandle,3);
        printf("%s (detach kernel)\n",libusb_error_name(errCheck));
        return NULL;
    }
    
    errCheck = libusb_claim_interface(devHandle, 3);
    if(errCheck != 0)
    {
        printf("%s (claim interface)\n", libusb_error_name(errCheck));
        errCheck = libusb_attach_kernel_driver(devHandle,3);
        if(errCheck != 0) printf("reattach kernel driver failed, reboot to fix");
        return NULL;
    }
    return devHandle;
}

int closeDevice(libusb_device_handle* handle)
{
    errCheck = libusb_release_interface(handle,3);
    if(errCheck != 0)
    {
        printf("%s\n (release interface)",libusb_error_name(errCheck));
        //return 1;
    }
    
    errCheck = libusb_attach_kernel_driver(handle,3);
    if(errCheck != 0)
    {
        printf("%s\n (attach kernel)",libusb_error_name(errCheck));
        //return 1;
    }

    libusb_close(handle);

    libusb_exit(NULL);

    return 0; 
}

int getProfileData(libusb_device_handle *handle, int profile, uint8_t *data)
{
    // PROFILE = 0 to 4
    char *profileString = "04008000";
    u_int8_t *profileData;
    errCheck = convertToDataArray(profileString,&profileData);
    if(errCheck != 0)
    {
        return 1;
    }
    profileData[1] = profile;
    
    /*
   Setup Data
    bmRequestType: 0x21
    bRequest: SET_REPORT (0x09)
    wValue: 0x0304
        ReportID: 4
        ReportType: Feature (3)
    wIndex: 3
    wLength: 4
    Data Fragment: 04 00 80 00
   */

    int errorCheck = libusb_control_transfer(handle,0x21,0x09,0x0304,0x0003,profileData,0x0004,1000); //SPECIFY WHICH PROFILE I WANT TO RETRIEVE
    if(errorCheck < 0){
        printf("Control transfer Failed\n");
        return 1;
    }
    /*
    Setup Data
    bmRequestType: 0xa1
    bRequest: GET_REPORT (0x01)
    wValue: 0x0306
        ReportID: 6
        ReportType: Feature (3)
    wIndex: 3
    wLength: 69
    */
    
    errorCheck = libusb_control_transfer(handle,0xa1,0x01,0x0306,0x0003,data,0x0045,1000); //RETRIEVE PROFILE DATA
    if(errorCheck < 0){
        printf("Control transfer Failed\n");
        return 1;
    }
    free(profileData);
    return 0;

}

void listProfileSettings(struct profile *p, int profile)
{
    uint8_t *listThisProfile;
    switch(profile)
    {
        case 0:
            listThisProfile = p->profile1;
            break;
        case 1:
            listThisProfile = p->profile2;
            break;
        case 2:
            listThisProfile = p->profile3;
            break;
        case 3:
            listThisProfile = p->profile4;
            break;
        case 4:
            listThisProfile = p->profile5;
            break;
        default:
            printf("Invalid profile\n");
            return;
    }

    //int dpi = (listThisProfile[7] * 50) + ((listThisProfile[8] * 256) * 50);
    printf("Profile %d\n", profile);
    printf("DPI: ");
    for(int i = 0,dpiSwitch = 0; dpiSwitch < 5;dpiSwitch++, i+=2)
    {
        printf("%d(Switch %d), ", (listThisProfile[7+i] * 50) + ((listThisProfile[8+i] * 256) * 50),dpiSwitch);
    }
    printf("\n");
    printf("Active DPI Switch: %d\n", listThisProfile[6]);
    printf("Left RGB: %d %d %d\n", listThisProfile[38],listThisProfile[39], listThisProfile[40]);
    printf("Right RGB: %d %d %d\n", listThisProfile[43],listThisProfile[44],listThisProfile[45]);
    printf("Polling Rate: %dHz\n", (int)(125 * (pow(2.0,(double)listThisProfile[29]))));
    printf("LED Mode: %d, (0 = 0ff, 1 = Fully lit, 2 = Blinking, 3 = Breathing, 4 = Heartbeat, 9 = Aimo Intelligent, 10 = Wave)\n", listThisProfile[30]);
    printf("LED Brightness: %d\n", listThisProfile[32]);
    printf("LED Speed: %d\n", listThisProfile[31]);
    printf("Debounce Time: %d ms (Global not profile specific)\n", p->dbt);
}

int setDebounceTime(libusb_device_handle *handle, uint8_t dbT)
{
    /*
    Setup Data
    bmRequestType: 0x21
    bRequest: SET_REPORT (0x09)
    wValue: 0x0311
    wIndex: 3
    wLength: 13
    Data Fragment: 110d0000000000000000001e00
    */
    if(dbT > 10) return 1;

    char *debounceString = "110d0000000000000000001e00";
    uint8_t *debounceData;
    errCheck = convertToDataArray(debounceString, &debounceData);
    if(errCheck != 0){
        printf("convertToDataArray Failed");
        return 1;
    }
    debounceData[2] = dbT; // Set debounce time 0 to 10
    int sum = 0;
    for(int i = 0; i < 11; i++){
        sum += debounceData[i];
    }
    debounceData[11] = sum; // Chekcsum
    int errorCheck = libusb_control_transfer(handle,0x21,0x09,0x0311,0x0003,debounceData,0x000d,1000);
    if(errorCheck < 0){
        printf("Control Transfer Failed\n");
        return 1;
    }
    free(debounceData);
    return 0;
}

uint8_t getDebounceTime(libusb_device_handle *handle)
{
   uint8_t dbtArray[13];

    /*
    Setup Data
    bmRequestType: 0xa1
    bRequest: GET_REPORT (0x01)
    wValue: 0x0311
    wIndex: 3
    wLength: 13
    */
   int errorCheck = libusb_control_transfer(handle,0xa1,0x01,0x0311,0x0003,dbtArray,0x000d,1000);
   if(errorCheck < 0){
        printf("Control Transfer Failed\n");
       return 11;
   }
   return dbtArray[2];
}
