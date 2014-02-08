#if !defined _MODBUS_INCLUDED
#define _MODBUS_INCLUDED

#define MODBUS_MAX_MSG_LENGTH	(91) // MAX(17, 11 + 4 * N), N - max words to read
#define MODBUS_READ_REQ_LENGTH	(17)
#define MODBUS_MAX_WORDS_READ	(20)

typedef enum
{
	MODBUS_READ = 3,
	MODBUS_WRITE = 6
} modbus_cmd_code_e;

typedef enum
{
	MODBUS_REQ = 0,
	MODBUS_ACK
} modbus_cmd_type_e;

typedef struct
{
/* 	device_id_e			device_id; */
	uint8_t				device_id;
	modbus_cmd_code_e	cmd_code;
	uint16_t			addr;
	uint16_t			value[MODBUS_MAX_WORDS_READ];
	modbus_cmd_type_e	cmd_type;
} modbus_cmd_s;

result_e modbus_byte2char(uint8_t /* byte */, uint8_t * /*buf */);
result_e modbus_char2byte(const uint8_t * /* buf */, uint8_t * /* byte */);
result_e modbus_word2char(uint16_t /* word */, uint8_t * /* buf */);
result_e modbus_char2word(const uint8_t * /* buf */, uint16_t * /* word */);
result_e modbus_crc(const uint8_t * /* msg */, uint16_t /* length */, uint8_t * /* crc */);
result_e modbus_cmd2msg(const modbus_cmd_s * /* cmd */, 
	uint8_t * /* msg */, uint16_t /* length */);
result_e modbus_msg2cmd(const uint8_t * /* msg */, modbus_cmd_s * /* cmd */);

#endif /* _MODBUS_INCLUDED */