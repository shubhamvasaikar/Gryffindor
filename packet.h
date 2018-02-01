#define PORT_NUMBER 5277
#define MAX_DATA 1024

typedef enum {
    REQ,
    ACK,
    FILE_REQ,
    FILE_REQ_ACK,
    DATA,
    DATA_ACK,
    TERM
}p_type;

typedef struct packet_t {
    p_type type;
    int seq_no;
    int length;
    uint8_t data[MAX_DATA];
} packet_t;

#define SEQ_OFFSET (sizeof(p_type))
#define LENGTH_OFFSET (sizeof(p_type) + sizeof(int))
#define DATA_OFFSET (sizeof(p_type) + sizeof(int) + sizeof(int))
#define META_SIZE (sizeof(p_type) + sizeof(int) + sizeof(int))
#define PACKET_SIZE (META_SIZE + MAX_DATA)

void encode(uint8_t *buffer, packet_t *packet) {
    memcpy(buffer, &(packet->type), sizeof(p_type));
    memcpy(buffer + SEQ_OFFSET, &(packet->seq_no), sizeof(int));
    memcpy(buffer + LENGTH_OFFSET, &(packet->length), sizeof(int));
    memcpy(buffer + DATA_OFFSET, &(packet->data), MAX_DATA);
}

void decode(uint8_t *buffer, packet_t *packet) {
    memcpy(&(packet->type), buffer, sizeof(p_type));
    memcpy(&(packet->seq_no), buffer + SEQ_OFFSET, sizeof(int));
    memcpy(&(packet->length), buffer + LENGTH_OFFSET, sizeof(int));
    memcpy(&(packet->data), buffer + DATA_OFFSET, MAX_DATA);
}

void encodeFilename(uint8_t *buffer, char *filename) {
    uint8_t len = (uint8_t) sizeof(filename);
    memcpy(buffer + LENGTH_OFFSET, &len, sizeof(uint8_t));
    memcpy(buffer + DATA_OFFSET, filename, strlen(filename));
}

void decodeFilename(uint8_t *buffer, char *filename) {
    uint8_t len = 0;
    memcpy(&len, buffer + LENGTH_OFFSET, 1);
    memcpy(filename, buffer + DATA_OFFSET + 1, len);
}