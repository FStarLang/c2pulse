#include <stdio.h>
#include <stdlib.h>

typedef struct _DEVICE_OBJECT DEVICE_OBJECT;
typedef struct _DEVICE_EXTENSION DEVICE_EXTENSION;

typedef struct _LIST_ENTRY {
    struct _LIST_ENTRY *Flink;
    struct _LIST_ENTRY *Blink;
} LIST_ENTRY, *PLIST_ENTRY;

// Inv: ALLOCATED DEVICE_OBJECT: DeviceExtension points to an ALLOCATED DEVICE_EXTENSION
// Inv: VALID DEVICE_OBJECT: DeviceExtension points to a VALID DEVICE_EXTENSION
struct _DEVICE_OBJECT {
    int OtherField1; 
    DEVICE_EXTENSION *DeviceExtension;
    int OtherField2;
    int OtherField3;
};

// Inv: VALID DEVICE_EXTENSION: Self points to a VALID DEVICE_OBJECT; Link is a doubly linked list entry
struct _DEVICE_EXTENSION {
    LIST_ENTRY Link;
    DEVICE_OBJECT *Self;
};

typedef struct _GLOBALS {
    // Inv: LegacyDeviceList is a doubly link list with Flink, Blink pointing to 
    //   a)  &LegacyDeviceList of the GLOBALS 
    //   b)  &Link of VALID DEVICE_EXTENSIONs that are not equal to GrandMaster
    LIST_ENTRY LegacyDeviceList;

    ///Inv: GrandMaster is either NULL or points to an VALID DEVICE_EXTENSION not in LegacyDeviceList
    DEVICE_EXTENSION *GrandMaster;

    // Inv: GLOBALS owns all DEVICE_EXTENSIONs in LegacyDeviceList and GrandMaster
} GLOBALS;

GLOBALS Globals;

// Initializes a doubly-linked list head
void InitializeListHead(PLIST_ENTRY ListHead) {
    ListHead->Flink = ListHead;
    ListHead->Blink = ListHead;
}

// Inserts an entry at the end of the list
void InsertTailList(PLIST_ENTRY ListHead, PLIST_ENTRY Entry) {
    PLIST_ENTRY Blink = ListHead->Blink;
    Entry->Flink = ListHead;
    Entry->Blink = Blink;
    Blink->Flink = Entry;
    ListHead->Blink = Entry;
}

// Removes an entry from a doubly-linked list
void RemoveEntryList(PLIST_ENTRY Entry) {
    Entry->Blink->Flink = Entry->Flink;
    Entry->Flink->Blink = Entry->Blink;
    Entry->Flink = Entry->Blink = Entry;
}

void InitalizeGlobals() {
    InitializeListHead(&Globals.LegacyDeviceList);
    Globals.GrandMaster = NULL;
}

DEVICE_OBJECT *IoCreateDevice() {
    DEVICE_OBJECT *devObj = (DEVICE_OBJECT *)malloc(sizeof(DEVICE_OBJECT));
    DEVICE_EXTENSION *devExt = (DEVICE_EXTENSION *)malloc(sizeof(DEVICE_EXTENSION));
    printf("Allocated DEVICE_OBJECT at %p\n", devObj);
    printf("Allocated DEVICE_EXTENSION at %p\n", devExt);
    devObj->DeviceExtension = devExt;
    return devObj;
}

void IoDeleteDevice(DEVICE_OBJECT *devObj) {
    DEVICE_EXTENSION *devExt = devObj->DeviceExtension;
    // free other resources allocated by IoCreateDevice
    printf("Freeing DEVICE_OBJECT at %p\n", devObj);
    printf("Freeing DEVICE_EXTENSION at %p\n", devExt);
    free(devExt);
    free(devObj);
}

// Unloads the driver and deletes all devices
void KeyboardClassUnload() {
    // Delete all of our legacy devices
    for (PLIST_ENTRY entry = Globals.LegacyDeviceList.Flink;
         entry != &Globals.LegacyDeviceList;){
        DEVICE_EXTENSION* data = (DEVICE_EXTENSION *) entry;
        RemoveEntryList(&data->Link);
        entry = entry->Flink;
        IoDeleteDevice(data->Self);
    }
    // Delete the grandmaster if it exists
    if (Globals.GrandMaster)
    {
        DEVICE_EXTENSION* data = Globals.GrandMaster;
        Globals.GrandMaster = NULL;
        IoDeleteDevice(data->Self);
    }
}

void KbdCreateClassObject(int isGrandMaster) {
    DEVICE_OBJECT *devObj = IoCreateDevice(); 
    DEVICE_EXTENSION *devExt = (DEVICE_EXTENSION *)devObj->DeviceExtension;

    // Inv: devObj is an ALLOCATED DEVICE_OBJECT
    InitializeListHead(&devExt->Link);    
    devExt->Self = devObj;

    //Inv: devObj is a VALID DEVICE_OBJECT
    if (isGrandMaster) {
        Globals.GrandMaster = devExt;
    } else {
        InsertTailList(&Globals.LegacyDeviceList, &devExt->Link);
    }

    // Inv: devObj is owned by GLOBALS
}

int main() {
    InitalizeGlobals();

    // Simulate creating devices
    for (int i = 0; i < 3; ++i) {
        KbdCreateClassObject(0); // Create legacy devices
    }

    KbdCreateClassObject(1); // Create the GrandMaster device

    // Now unload and delete everything
    KeyboardClassUnload();

    // Inv: all allocated DEVICE_OBJECTs and DEVICE_EXTENSIONs have been freed
    printf("Devices deleted.\n");
    return 0;
}
