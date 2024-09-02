#include "OD.h"

/* Hash function for numbers */
unsigned int hash(uint16_t index, uint8_t subindex, unsigned int table_size) {
    unsigned int hash_value = index;
    hash_value = hash_value * 31 + subindex; // 31 is a prime number
    return hash_value % table_size;
}

/* 
Allocates the required memory for the object 
dictionary and returns pointer 
*/
can_od_t* initOD(int numberObjects){
    can_od_t* OD = (can_od_t*)malloc(sizeof(can_od_t));
    OD->odObjects = (can_od_object_t*)malloc(
        numberObjects * sizeof(can_od_object_t));
    OD->numberObjects = numberObjects;
    if(OD->odObjects == NULL){
        printf("ERROR: Memory allocation of object dictionary failed");
        free(OD);
        return NULL;
    }
    for(int i = 0; i < numberObjects; i++){
        OD->odObjects[i].value = NULL;
    }
    return OD;
}

/* Allocates the memory according to the CAN_DATA_TYPE */
void* allocateValue(CAN_DATA_TYPE dataType){

    void* valuePtr = NULL;
    switch(dataType){
        case BOOLEAN:
        case UNSIGNED8:
            valuePtr = (uint8_t*)malloc(sizeof(uint8_t));
            break;
        case UNSIGNED16:
            valuePtr = (uint16_t*)malloc(sizeof(uint16_t));
            break;
        case UNSIGNED32:
            valuePtr = (uint32_t*)malloc(sizeof(uint32_t));
            break;
        case INTEGER8:
            valuePtr = (int8_t*)malloc(sizeof(int8_t));
            break;
        case INTEGER16:
            valuePtr = (int16_t*)malloc(sizeof(int16_t));
            break;
        case INTEGER32:
            valuePtr = (int32_t*)malloc(sizeof(int32_t));
            break;
    }
    return valuePtr;
}

/* Writes the data to the pointer with the right datat type */
void writeValue(void* valuePtr, uint16_t dataType, int value) {
    switch (dataType) {
        case BOOLEAN:
        case UNSIGNED8:
            *(uint8_t*)valuePtr = (uint8_t)value;
            break;
        case UNSIGNED16:
            *(uint16_t*)valuePtr = (uint16_t)value;
            break;
        case UNSIGNED32:
            *(uint32_t*)valuePtr = (uint32_t)value;
            break;
        case INTEGER8:
            *(int8_t*)valuePtr = (int8_t)value;
            break;
        case INTEGER16:
            *(int16_t*)valuePtr = (int16_t)value;
            break;
        case INTEGER32:
            *(int32_t*)valuePtr = (int32_t)value;
            break;
        default:
            printf("Error: Unsupported data type\n");
            break;
    }
}

/* 
Adds an can_od_object_t object to can_od_t 
and stores it to the position of the hash 
*/
void insertObject(
    can_od_t* OD, 
    uint16_t index, 
    uint8_t subindex, 
    uint16_t dataType, 
    uint8_t access,
    uint8_t persistence,
    int value){

    int key = hash(index, subindex, OD->numberObjects);
    void* valuePtr = allocateValue(dataType);

    if(valuePtr == NULL){
        printf("Error: Object Dictionary value could not be allocated\n");
    }

    if (OD->odObjects[key].value == NULL){
        /* Memory is still unused and value can be written */
        printf("INFO: Writing to OD object %X.%X.h\n", index, subindex);

    } else if (OD->odObjects[key].value != NULL &&
               OD->odObjects[key].index != index){
        /* Hash function creates the same hash for two different index values */
        printf("Error: Hash collision. \
                Collision handling not implemented yet.\n");
        return;
    } else if(OD->odObjects[key].value != NULL 
            && OD->odObjects[key].index == index 
            && OD->odObjects[key].subindex == subindex){
        /* Object already exists and will be overwritten */
        printf("Warning: Object %X.%Xh was inserted multiple times. \
            Values will be overwritten.\n",
            index, subindex);
    } 

    writeValue(valuePtr, dataType, value);

    OD->odObjects[key].index = index;
    OD->odObjects[key].subindex = subindex;
    OD->odObjects[key].dataType = dataType;
    OD->odObjects[key].access = access;
    OD->odObjects[key].persistence = persistence;
    OD->odObjects[key].value = valuePtr;
}

/* Frees up the allocated memory of the full object dictionary */
void freeOD(can_od_t* OD){
    for(int i = 0; i < OD->numberObjects; i++){
        if(OD->odObjects[i].value != NULL){
            free(OD->odObjects[i].value);
        }
    }
    free(OD->odObjects);
}

/* Retuns the pointer of the defined OD object */
void* getODValue(can_od_t* OD, uint16_t index, uint16_t subindex){
    int key = hash(index, subindex, OD->numberObjects);
    if(OD->odObjects[key].value){
        void* value = OD->odObjects[key].value;
        return value; 
    } else { 
        printf("ERROR: Object %X.%X not supported\n",index, subindex);
        return NULL;
    } 
}

/* Change the value of an object dictionary object */
void setODValue(can_od_t* OD, uint16_t index, uint16_t subindex, int value){
    int key = hash(index, subindex, OD->numberObjects);
    if(OD->odObjects[key].value == NULL){
        printf("ERROR: OD object %X.%Xh is not supported\n", index, subindex);
    }
    writeValue(OD->odObjects[key].value, OD->odObjects[key].dataType, value);
}



