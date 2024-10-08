#include "CANopen.h"

/* Extern global variables */
extern uint8_t reset;

/* Global variables */
uint8_t tpdo_sync_counter[4] = {0, 0, 0, 0};

/* Prints a can tx message to the standard output*/
void can_print_tx_message(twai_message_t *msg){
    printf("TX MESSAGE ID: %lX, DLC: %d, DATA: [0x%X, 0x%X, 0x%X, 0x%X, 0x%X, 0x%X, 0x%X, 0x%X]\n", 
    msg->identifier,
    msg->data_length_code,
    msg->data[0],
    msg->data[1],
    msg->data[2],
    msg->data[3],
    msg->data[4],
    msg->data[5],
    msg->data[6],
    msg->data[7]
    );
}

/* Prints a can tx message to the standard output*/
void can_print_rx_message(twai_message_t *msg){
    printf("RX MESSAGE ID: %lX, DLC: %d, DATA: [0x%X, 0x%X, 0x%X, 0x%X, 0x%X, 0x%X, 0x%X, 0x%X]\n", 
    msg->identifier,
    msg->data_length_code,
    msg->data[0],
    msg->data[1],
    msg->data[2],
    msg->data[3],
    msg->data[4],
    msg->data[5],
    msg->data[6],
    msg->data[7]
    );
}

/* Transmits and prints the message to stdout*/
void can_transmit(twai_message_t *msg){
    ESP_ERROR_CHECK(twai_transmit(msg, TX_TIMEOUT));
    can_print_tx_message(msg);
};

/* Config the CAN module */
void can_config_module(void){
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX, CAN_RX, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = CAN_BAUD_RATE;
    twai_filter_config_t f_config = {
        .acceptance_mask = CAN_ACCEPTANCE_MASK,
        .acceptance_code = CAN_ACCEPTANCE_CODE,
        .single_filter = CAN_SINGLE_FILTER
    };
    

    /* This should improve number of error frames on install */
    gpio_set_direction(CAN_TX, GPIO_MODE_OUTPUT);
    gpio_set_level(CAN_TX, 1);
    vTaskDelay(pdMS_TO_TICKS(10));

    // Uninstalled to stopped state
    printf("INFO: Installing CAN module... ");
    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        printf("Done\n");
        }
    else{
        printf("\nERROR: Failed to install CAN module\n");
    }
}

/* Starts the CAN module */
void can_start_module(void){
    printf("INFO: Starting CAN module... \n");
    if (twai_start() == ESP_OK) {
        printf("Done\n");
    }
    else {
        printf("\nERROR: Failed to start CAN module\n");
        }
}

/* Stops the can module */
void can_stop_module(void){
    printf("INFO: Stopping CAN module... ");
    if(twai_stop() == ESP_OK){
        printf("Done\n");
    } else {
        printf("\nERROR: Could not stop CAN module\n");
    }

}

/* Uninstalls module and frees memory */
void can_uninstall_module(void){
    printf("INFO: Uninstalling CAN module... ");
    if(twai_driver_uninstall() == ESP_OK){
        printf("Done\n");
    } else {
        printf("\nERROR: CAN module could not be uninstalled\n");
    }
}

/* Initializes CAN node and sets NMT state to pre-operational */
void can_node_init(can_node_t *node){
    if(node){
        node->id = CAN_NODE_ID;
        node->nmtState = CAN_NMT_INITIALIZING;
        if(CAN_NODE_ID < 128){
            node->lssMode = LSS_OPERATION_MODE;
            node->nmtState = CAN_NMT_PRE_OPERATIONAL;
        } else {
            node->lssMode = LSS_CONFIG_MODE;
        }
    } else {
        printf("ERROR: Invalid node cannot set to pre-operational\n");
        return;
    };
}

/* Get*/
uint8_t extract_uint8(twai_message_t *msg, uint8_t startbyte){
    uint8_t value = 0;
    value = msg->data[startbyte];
    return value;
}

uint16_t extract_uint16(twai_message_t *msg, uint8_t startbyte){
    uint16_t value = 0;
    for(uint8_t i = 0; i < 2; i++){
        value |= (msg->data[startbyte+i] << (8 * i));
    }
    return value;
}

uint32_t extract_uint24(twai_message_t *msg, uint8_t startbyte){
    uint32_t value = 0;
    for(uint8_t i = 0; i < 3; i++){
            value |= (msg->data[startbyte+i] << (8 * i));
    }
    return value;
}

uint32_t extract_uint32(twai_message_t *msg, uint8_t startbyte){
    uint32_t value = 0;
    for(uint8_t i = 0; i < 4; i++){
            value |= (msg->data[startbyte+i] << (8 * i));
    }
    return value;
}

uint64_t extract_uint64(twai_message_t *msg, uint8_t startbyte){
    uint64_t value = 0;
    for(uint8_t i = 0; i < 8; i++){
            value |= (msg->data[startbyte+i] << (8 * i));
    }
    return value;
}

void insert_uint8(twai_message_t *msg,  uint8_t startbyte, uint32_t* value){
    if(value == NULL){
        printf("ERROR: Unsupported object cannot be changed\n");
        return;

        uint8_t byte = *(uint32_t*)value & 0xFF;
        msg->data[startbyte] = byte;
    }
}

void insert_uint16(twai_message_t *msg,  uint8_t startbyte, uint32_t* value){
    if(value == NULL){
        printf("ERROR: Unsupported object cannot be changed\n");
        return;
    }
    for(uint8_t i=0; i<2; i++){
        uint8_t byte = (*(uint32_t*)value >> (i * 8)) & 0xFF;
        msg->data[startbyte+i] = byte;
    }
}

void insert_uint24(twai_message_t *msg,  uint8_t startbyte, uint32_t* value){
    if(value == NULL){
        printf("ERROR: Unsupported object cannot be changed\n");
        return;
    }
    for(uint8_t i=0; i<3; i++){
        uint8_t byte = (*(uint32_t*)value >> (i * 8)) & 0xFF;
        msg->data[startbyte+i] = byte;
    }
}

void insert_uint32(twai_message_t *msg,  uint8_t startbyte, uint32_t* value){
    if(value == NULL){
        printf("ERROR: Unsupported object cannot be changed\n");
        return;
    }
    for(uint8_t i=0; i<4; i++){
        uint8_t byte = (*(uint32_t*)value >> (i * 8)) & 0xFF;
        msg->data[startbyte+i] = byte;
    }
}

/* Initializes the non volatile storage of esp */
void init_nvs(can_node_t *node, nvs_handle_t *nvsHandle){
    esp_err_t err;
    // Initialize NVS
    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // Open NVS
    printf("INFO: Opening Non-Volatile Storage handle... ");
    err = nvs_open("OD_PERS", NVS_READWRITE, nvsHandle);
    if (err != ESP_OK) {
        printf("\nError (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        printf("Done\n");
    }
    node->OD->nvsHandle = *nvsHandle;
}

/* 
Stores the current configuration to non volatile stoarge
*/
void store_OD_persistent(can_node_t *node, nvs_handle_t *nvsHandle){
    esp_err_t err;
    uint32_t indexAndSubindex;
    uint32_t volValue;
    uint32_t persValue;
    uint16_t index;
    uint8_t subindex;
    char keyStr[7];
    for(int i=0; i < node->OD->numberPersistentObjects; i++){
        indexAndSubindex = (uint32_t)node->OD->persistentObjectIds[i];
        index = (indexAndSubindex >> 8) & 0xFFFF;
        subindex = (indexAndSubindex >> 0) & 0xFF;

        sprintf(keyStr, "%X%X", index, subindex);
        
        // Get volatile OD object value
        can_od_object_t* object = getODentry(node->OD, index, subindex);
        volValue = *(uint32_t*)object->value;

        err = nvs_set_u32(*nvsHandle, keyStr, volValue);

        nvs_commit(*nvsHandle); 
        if(err!=ESP_OK){
            printf("ERROR: While NVS write (%s)\n", esp_err_to_name(err));
            return;
        } else {
            /* Check if persistent value was written */
            nvs_get_u32(*nvsHandle, keyStr, &persValue);
            printf("INFO: Persistent value %X.%Xh set to %lu\n", 
            index, subindex, persValue);  
        }
    }
    
}

/* Loads the values from the persistent data and stores it to volatile */
void load_OD_persistent(can_node_t *node, nvs_handle_t *nvsHandle){
    esp_err_t err;
    uint32_t indexAndSubindex;
    uint32_t persValue;
    uint16_t index;
    uint8_t subindex;
    char keyStr[7];
    for(int i=0; i < node->OD->numberPersistentObjects; i++){
        indexAndSubindex = (uint32_t)node->OD->persistentObjectIds[i];
        index = (indexAndSubindex >> 8) & 0xFFFF;
        subindex = (indexAndSubindex >> 0) & 0xFF;
        // NVS requires strings as keys, so the index and subindex
        sprintf(keyStr, "%X%X", index, subindex);
        err = nvs_get_u32(*nvsHandle, keyStr, &persValue);
        if(err == ESP_OK){
            printf("INFO: Persistent value %X.%Xh=%lu\n", 
                    index, subindex, persValue);
        } else {
            printf("ERROR: While NVS read (%s)\n", esp_err_to_name(err));
            return;
            }
        
        // Get volatile OD object value
        can_od_object_t* object = getODentry(node->OD, index, subindex);
        *(object->value) = persValue;
    }
    
}

/* Sends response after chaning NMT state with current state */
void send_nmt_state(can_node_t *node){

    if(CAN_NODEGUARD && node->rxMsg.identifier == (CAN_HB_ID + node->id)){
        node->nmtState ^= 1 << CAN_NODEGUARD_TOGGLE_BIT;
    }
    node->txMsg.identifier =  (CAN_HB_ID + node->id);
    node->txMsg.data_length_code = 1;
    node->txMsg.data[0] = node->nmtState;


    ESP_ERROR_CHECK(twai_transmit(&node->txMsg, TX_TIMEOUT));
    can_print_tx_message(&node->txMsg);
};

/* Sets the whole can message data to 0*/
void empty_msg_data(twai_message_t* msg){
    for(int i = 0; i < 8; i++){
        msg->data[i] = 0;
    }
}

void sdo_write_success(can_node_t *node){
    node->txMsg.data[0] = SDO_DOWNLOAD_SUCCESS;
    node->txMsg.data[1] = node->rxMsg.data[1];
    node->txMsg.data[2] = node->rxMsg.data[2];
    node->txMsg.data[3] = node->rxMsg.data[3];
    node->txMsg.data_length_code = 8;
    ESP_ERROR_CHECK(twai_transmit(&node->txMsg, TX_TIMEOUT));
    can_print_tx_message(&node->txMsg);
}

/* Handles a write command to 1010h */
void store_parameters_service(can_node_t *node){
    twai_message_t rxMsg = node->rxMsg;
    empty_msg_data(&node->txMsg);
    node->txMsg.data_length_code = 8;
    node->txMsg.data[0] = 4;
    node->txMsg.data[1] = rxMsg.data[1];
    node->txMsg.data[2] = rxMsg.data[2];
    node->txMsg.data[3] = rxMsg.data[3];
 
    if(rxMsg.data[3] == 1){
        /* Check if signature is "save" */
        uint32_t signature = extract_uint32(&rxMsg, 4);
        if(signature == SDO_SAVE_SIGNATURE){
            store_OD_persistent(node, &node->OD->nvsHandle);
        }
    } 
    /* In all other cases abort the service */
    uint32_t abortCode = 0x08000020;
    insert_uint32(&node->txMsg, 4, &abortCode);
    ESP_ERROR_CHECK(twai_transmit(&node->txMsg, TX_TIMEOUT));
    can_print_tx_message(&node->txMsg);
}
/* Writes the value to the sdo object */
void write_sdo_object(can_node_t *node){
    empty_msg_data(&node->txMsg);
    uint16_t index = extract_uint16(&node->rxMsg, 1);
    uint8_t subindex = extract_uint8(&node->rxMsg, 3);
    can_od_object_t* object = getODentry(node->OD, index, subindex);
    if(object->value == NULL){
        printf("ERROR: Attempt to write to unsupported object\n");
        node->txMsg.data[0] = SDO_ABORT;
        ESP_ERROR_CHECK(twai_transmit(&node->txMsg, TX_TIMEOUT));
        can_print_tx_message(&node->txMsg);
        return;
    } else if((object->access == READ_ONLY) | (object->access == CONST)){
        printf("ERROR: Attempt to write to read only value\n");
        node->txMsg.data[0] = SDO_ABORT;
        ESP_ERROR_CHECK(twai_transmit(&node->txMsg, TX_TIMEOUT));
        can_print_tx_message(&node->txMsg);
        return;
    }
    uint8_t dlc = node->rxMsg.data_length_code;
    uint32_t value = 0;
    switch(dlc){
        case 5: // data one byte
            value = extract_uint8(&node->rxMsg, 4);
            break;
        case 6: // data two bytes
            value = extract_uint16(&node->rxMsg, 4);
            break;
        case 7: // data three bytes
            value = extract_uint24(&node->rxMsg, 4);
            break;
        case 8: // data four bytes
            value = extract_uint32(&node->rxMsg, 4);
            break;
    }
    if(index != 0x1010){
        *(uint32_t*)object->value = value;
        printf("INFO: %X.%X changed to %lu\n", index, subindex, value);
    } else {
        store_parameters_service(node);
        return; 
    }
    sdo_write_success(node);
}

/* Sends data of current sdo object */
void send_sdo_object(can_node_t *node){
    empty_msg_data(&node->txMsg);

    /* Prepare right response byte */
    uint8_t sdoResponse = 0;
    uint8_t sdoCommand = node->rxMsg.data[0];
    uint8_t numberEmptyBytes = (sdoCommand & SDO_N_MASK) >> 2;
    switch(numberEmptyBytes){
        case 0:
            sdoResponse = SDO_UPLOAD_4_BYTES;
            break;
        case 1:
            sdoResponse = SDO_UPLOAD_3_BYTES;
            break;
        case 2: 
            sdoResponse = SDO_UPLOAD_2_BYTES;
            break;
        case 3:
            sdoResponse = SDO_UPLOAD_1_BYTE;
    }
    if (numberEmptyBytes > 0){
        if((sdoCommand & SDO_S_MASK) == 0 || (sdoCommand & SDO_E_MASK) == 0){
            sdoResponse = SDO_ABORT;
           }
    }

    node->txMsg.identifier = CAN_TX_SDO + node->id;
    node->txMsg.data_length_code = 8;

    /* Insert index and subindex into response*/
    
    node->txMsg.data[1] = node->rxMsg.data[1];
    node->txMsg.data[2] = node->rxMsg.data[2];
    node->txMsg.data[3] = node->rxMsg.data[3];


    uint16_t index = extract_uint16(&node->rxMsg, 1);
    uint8_t subindex = extract_uint8(&node->rxMsg, 3);
    can_od_object_t* object = getODentry(node->OD, index, subindex);
    uint32_t value;
    switch(object->dataType){
        case UNSIGNED8:
        case INTEGER8: 
            value = *(uint8_t*)object->value;
            break;
        case UNSIGNED16: 
        case INTEGER16:
            value = *(uint16_t*)object->value;
            break;
        case UNSIGNED32:
        case INTEGER32: 
            value = *(uint32_t*)object->value;
            break;
        default:
            printf("ERROR: Data type not supported by SDO service\n");
            break;
    }
    if(object->value==NULL){
        sdoResponse = SDO_ABORT;
    } else {
        insert_uint32(&node->txMsg, 4, &value);
    }
    node->txMsg.data[0] = sdoResponse;
    ESP_ERROR_CHECK(twai_transmit(&node->txMsg, TX_TIMEOUT));
    can_print_tx_message(&node->txMsg);
};

/* SDO service to handle SDO frames */
void sdo_service(can_node_t *node){
    uint8_t sdoCommand = node->rxMsg.data[0];
    if((sdoCommand & SDO_CSS_MASK) == SDO_INIT_EXP_UPLOAD){
        send_sdo_object(node);
    } else if((sdoCommand & SDO_CSS_MASK) == SDO_INIT_EXP_DOWNLOAD)
        write_sdo_object(node);
}

/* Build the message for the tpdo transfer */
void build_and_send_tpdo(can_node_t* node, 
                uint16_t communicationObjectId,
                uint16_t mappingObjectId,
                uint8_t tpdoNumber){

    empty_msg_data(&node->txMsg);
    can_od_object_t* mapping0 = getODentry(node->OD, mappingObjectId, 0);
    int nrObjects = (*(uint8_t*)mapping0->value)+1;
    uint8_t currentPosition = 0;
    for(int i = 1; i < nrObjects; i++){
        can_od_object_t* mappingx = getODentry(node->OD, mappingObjectId, i);
        uint32_t value = *(uint32_t*)mappingx->value;
        uint16_t objIndex = (value >> 16) & 0xFFFF;
        uint8_t objSubindex = (value >> 8) & 0xFF;
        uint8_t objLength = value & 0xFF;
        //printf("Object length: %u\n", objLength);
        can_od_object_t* dataObjectx = getODentry(node->OD, objIndex, objSubindex);
        uint32_t* objValue = (uint32_t*)dataObjectx->value;
        //printf("%X.%Xh: %lu\n", objIndex, objSubindex,*(uint32_t*)objValue);
        switch(objLength){
            case 1:
                insert_uint8(&node->txMsg, currentPosition, objValue);
                currentPosition++;
                break;
            case 2:
                insert_uint16(&node->txMsg, currentPosition, objValue);
                currentPosition += 2;
                break;
            case 3:
                insert_uint24(&node->txMsg, currentPosition, objValue);
                currentPosition += 3;
                break;
            case 4:
                insert_uint32(&node->txMsg, currentPosition, objValue);
                currentPosition += 4;
                break;
            default:
                printf("ERROR: Invalid data length defined in TPDO mapping\n");

        }    
    }
    node->txMsg.data_length_code = 8;
    uint16_t cobId = (uint16_t)getODValue(node->OD, communicationObjectId,1);
    node->txMsg.identifier = cobId;

    /* Check if tpdo should be sent based on the number of sync counter */
    uint32_t transmissionType = getODValue(node->OD, communicationObjectId, 2);
    if((transmissionType >= 0x1) & (transmissionType <= 0xF0)){
        if(tpdo_sync_counter[tpdoNumber] == transmissionType){
            can_transmit(&node->txMsg);
            tpdo_sync_counter[tpdoNumber]=0; // reset to zero after transmit
        }

    } else {
        printf("ERROR: Transmission type '%lu'of tpdo not supported\n", 
            transmissionType);
    }
}

/* 
TPDO service to handle sync messages. 
Note: Usually PDOs can be also sent on timeouts, but in this application
it will only use TPDOX on SYNC. 
*/
void tpdo_service(can_node_t *node){
    /* PDO messages only in operational mode*/
    if((node->nmtState & 0x7F) != CAN_NMT_OPERATIONAL){
        return;
    }
    uint16_t tpdo_parameter[4] = {OD_TPDO1_PARAMETER, OD_TPDO2_PARAMETER,
        OD_TPDO3_PARAMETER, OD_TPDO4_PARAMETER};
    uint16_t tpdo_mapping[4] = {OD_TPDO1_MAPPING, OD_TPDO2_MAPPING,
        OD_TPDO3_MAPPING, OD_TPDO4_MAPPING};

    for(int i = 0; i < 4; i++){
        uint8_t parameter_object = (uint8_t)getODValue(node->OD, tpdo_parameter[i],0);
        if(parameter_object > 0){
            tpdo_sync_counter[i]++;
            build_and_send_tpdo(node, (uint16_t)tpdo_parameter[i],(uint16_t)tpdo_mapping[i], (uint8_t)i);
        }
    }
}

void change_nmt(can_node_t *node){
    uint8_t targetNode = node->rxMsg.data[1];
    if(targetNode == node->id || targetNode == 0){
        switch(node->rxMsg.data[0]){
        case CAN_NMT_CMD_OPERATIONAL:
            node->nmtState = CAN_NMT_OPERATIONAL;
            break;
        case CAN_NMT_CMD_PRE_OPERATIONAL:
            node->nmtState = CAN_NMT_PRE_OPERATIONAL;
            break;
        case CAN_NMT_CMD_RESET_COMMUNICATION:
            // TODO: implement case
            break;
        case CAN_NMT_CMD_RESET_NODE:
            reset = CAN_RESET;
            return;
        case CAN_NMT_CMD_STOP:
            node->nmtState = CAN_NMT_STOPPED;
            break;
        default:
            return;
        }
    }
}

/* Impelementation for selective switch */
void lss_switch_selective(can_node_t *node){
    twai_message_t rxMsg = node->rxMsg;
    uint16_t index = 0x1018;
    uint8_t subindex;
    uint32_t messageValue = extract_uint32(&rxMsg, 1);
    printf("IDENTITY OBJECT: %lu\n", messageValue);
    uint32_t comparisonValue;
    switch(rxMsg.data[0]){
        case LSS_CS_SELECTIVE_VENDOR:
            subindex=1;
            break;
        case LSS_CS_SELECTIVE_PRODUCT:
            subindex=2;
            break;
        case LSS_CS_SELECTIVE_REVISION:
            subindex=3;
            break;
        case LSS_CS_SELECTIVE_SERIAL:
            subindex=4;
            break;
        default:
            return;
    }
    // Get values of identity object for the active switch
    can_od_object_t* object = getODentry(node->OD, index, subindex);
    comparisonValue = *(uint32_t*)object->value;
    if(comparisonValue == messageValue){
        node->lssMode |= 1 << (subindex-1);
        printf("LSS MODE: %u\n", node->lssMode);
    }
    if(node->lssMode == LSS_CONFIG_MODE){
        empty_msg_data(&node->txMsg);
        node->txMsg.identifier = LSS_TX_COB_ID;
        node->txMsg.data_length_code = 8;
        node->txMsg.data[0] = LSS_CS_SELCTIVE_RESPONSE;
        printf("INFO: LSS -> Node set to configuration mode\n");
        twai_transmit(&node->txMsg, TX_TIMEOUT);
        can_print_tx_message(&node->txMsg);
    }

}

/* LSS service to change the node id of the device */
void lss_config_node(can_node_t *node){
    if(node->lssMode == LSS_CONFIG_MODE){
        empty_msg_data(&node->txMsg);
        node->txMsg.identifier = LSS_TX_COB_ID;
        node->txMsg.data_length_code = 8;
        node->txMsg.data[0] = LSS_CS_CONFIG_NODE;
        uint8_t newNodeId = node->rxMsg.data[1];
        if(newNodeId<128){
            can_od_object_t* nodeIdObject = getODentry(node->OD, OD_NODE_ID, 0);
            *(uint32_t*)nodeIdObject->value = (uint32_t)newNodeId;
            node->txMsg.data[1] = LSS_NODE_SUCCESS;
            printf("INFO: LSS -> Node ID set to %u\n", newNodeId);   
        } else {
            node->txMsg.data[1] = LSS_ERR_ID_OUT_OF_RANGE;
        }
        twai_transmit(&node->txMsg, TX_TIMEOUT);
        can_print_tx_message(&node->txMsg);
    }
}

/* 
LSS service to store the configuration in non volatile memory.
This will also lead to a restart of the node. 
*/
void lss_store_config(can_node_t *node){
    empty_msg_data(&node->txMsg);
    node->txMsg.data_length_code = 8;
    node->txMsg.identifier = LSS_TX_COB_ID;
    node->txMsg.data[0] = LSS_CS_STORE_CONFIG;
    store_OD_persistent(node, &node->OD->nvsHandle);
    /* 
    TODO: Continue here - change the storage function to a function that only
    saves the node id. Also send a response message after storage with CS  
    and error code. Also make an error check and respond with error otherwise.
    */ 
   twai_transmit(&node->txMsg, TX_TIMEOUT);
   can_print_tx_message(&node->txMsg);
}
/* Returns the current node id via the lss protocol */
void lss_return_node(can_node_t *node){
    empty_msg_data(&node->txMsg);
    node->txMsg.data_length_code = 2;
    node->txMsg.identifier = LSS_TX_COB_ID;
    node->txMsg.data[0] = LSS_ASK_NODE;
    node->txMsg.data[1] = node->id;
    twai_transmit(&node->txMsg, TX_TIMEOUT);
    can_print_tx_message(&node->txMsg);
}

/* Processes the LSS service such as node id change and baudrate change */
void lss_service(can_node_t *node){
    switch(node->rxMsg.data[0]){
        case LSS_CS_SELECTIVE_VENDOR:
        case LSS_CS_SELECTIVE_PRODUCT:
        case LSS_CS_SELECTIVE_REVISION:
        case LSS_CS_SELECTIVE_SERIAL:
            // Initialization of selective switch 
            lss_switch_selective(node);
            break;
        case LSS_CS_CONFIG_NODE:
            lss_config_node(node);
            break;
        case LSS_CS_STORE_CONFIG:
            lss_store_config(node);
            break;
        case LSS_ASK_NODE:
            lss_return_node(node);
            break;
        default:
            printf("ERROR: LSS command not defined\n");
    }
}

/* Processes the incoming message and sends response if needed */
void can_process_message(can_node_t *node){
    uint32_t identifier = node->rxMsg.identifier;
    if (identifier == CAN_SYNC){
        tpdo_service(node);
        return;
    } else if (identifier == CAN_NMT_ID){
        change_nmt(node);
        return;
    } else if (identifier == (CAN_HB_ID + node->id)){
        /* HEARTBEAT PROTOCOL*/
        send_nmt_state(node);
        return;
    } else if (identifier == (CAN_RX_SDO + node->id)){
        sdo_service(node);
        return;
    } else if (identifier == LSS_RX_COB_ID){
        lss_service(node);
    }
}

/* Updates the node id according to the NVS value */
void update_node_id(can_node_t *node){
    can_od_object_t* nodeIdObj = getODentry(node->OD, OD_NODE_ID, 0);
    uint32_t newNodeId = *(uint32_t*)nodeIdObj->value;
    node->id = newNodeId & 0xFF;

    // Update also the SDOs that are affected
    can_od_object_t* odEntry = getODentry(node->OD, OD_TPDO1_PARAMETER, 0x1);
    *(uint32_t*)odEntry->value = 0x180 + node->id;
    odEntry = getODentry(node->OD, OD_TPDO2_PARAMETER, 0x1);
    *(uint32_t*)odEntry->value = 0x280 + node->id;
}
    
