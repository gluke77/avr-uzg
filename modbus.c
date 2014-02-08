#include <avr\io.h>
#include "common.h"
#include "modbus.h"
#include <string.h>
#if defined _MASTER
	#include "lcd.h"
#endif /* _MASTER */

/*****************************************************************************/

result_e modbus_bits2char(uint8_t bits, uint8_t * buf)
{
	bits &= 0x0F;
	*buf = (bits < 0x0A)?('0' + bits):('A' - 0x0A + bits);
	return RESULT_OK;
}

/*****************************************************************************/

result_e modbus_byte2char(uint8_t byte, uint8_t * buf)
{
	modbus_bits2char(byte >> 4, buf);
	modbus_bits2char(byte & 0x0F, buf + 1);
	return RESULT_OK;
}

/*****************************************************************************/

result_e modbus_word2char(uint16_t word, uint8_t * buf)
{
	modbus_byte2char(word >> 8, buf);
	modbus_byte2char((uint8_t)(word & 0x00FF), buf + 2);
	return RESULT_OK;
}

/*****************************************************************************/

result_e modbus_char2bits(const uint8_t ch, uint8_t * bits)
{
	if ('A' <= ch && ch <= 'F')
		*bits = ch - 'A' + 0x0A;
	else if ('0' <= ch && ch <= '9' )
		*bits = ch - '0';
	else
	{
		return RESULT_BAD_MSG;
	}

	return RESULT_OK;
}

/*****************************************************************************/

result_e modbus_char2byte(const uint8_t * buf, uint8_t * byte)
{
	result_e	res;
	uint8_t			tmp;
	
	if ((res = modbus_char2bits(*buf, &tmp)) != RESULT_OK)
		return res;
		
	*byte = tmp << 4;
	
	res = modbus_char2bits(*(buf + 1), &tmp);
	
	*byte += tmp;
	
	return res;
}

/*****************************************************************************/

result_e modbus_char2word(const uint8_t * buf, uint16_t * word)
{
	result_e	res;
	uint8_t		tmp;
	
	if ((res = modbus_char2byte(buf, &tmp)) != RESULT_OK)
		return res;
		
	*word = (uint16_t)tmp << 8;
	
	res = modbus_char2byte(buf + 2, &tmp);
	
	*word += (uint16_t)tmp;
	
	return res;
}

/*****************************************************************************/

result_e modbus_crc(const uint8_t * msg, uint16_t length, uint8_t * crc)
{
	uint8_t		acc;
	uint8_t 	byte;
	uint16_t	tmp;
	result_e	res;
	
	for (acc = 0, tmp = 0; tmp < length; tmp += 2, acc += byte)
		if ((res = modbus_char2byte(msg + tmp, &byte)) != RESULT_OK)
			return res;
	
	modbus_byte2char((~acc)+1, crc);
	
	return RESULT_OK;
}

/*****************************************************************************/

result_e modbus_cmd2msg(const modbus_cmd_s * cmd, uint8_t * msg, uint16_t length)
{
	uint16_t	cmd_size;
	uint8_t		idx;
	result_e	res;
	uint8_t 	* pos;
	
	cmd_size = (cmd->cmd_type == MODBUS_ACK && cmd->cmd_code == MODBUS_READ)?MODBUS_MAX_MSG_LENGTH:17;
	
	if (length < cmd_size)
		return RESULT_BUFFER_OVERFLOW;
		
	pos = msg;
	
	*(pos++) = ':';
	
	modbus_byte2char(cmd->device_id, pos);
	pos += 2;
	
	modbus_byte2char((uint8_t)cmd->cmd_code, pos);
	pos += 2;
	
	if (cmd->cmd_type == MODBUS_ACK && cmd->cmd_code == MODBUS_READ)
	{
		modbus_byte2char((uint8_t)((cmd->addr * 2) & 0x00FF), pos);
		pos += 2;
	}
	else
	{
		modbus_word2char(cmd->addr, pos);
		pos += 4;
	}
	
	modbus_word2char(cmd->value[0], pos);
	pos += 4;
	
	if (cmd->cmd_type == MODBUS_ACK && cmd->cmd_code == MODBUS_READ)
		for (idx = 1; idx < cmd->addr; idx++)
		{
			modbus_word2char(cmd->value[idx], pos);
			pos += 4;
		}
	
	
	if ((res = modbus_crc(msg + 1, (uint16_t)(pos - msg - 1), pos)) != RESULT_OK)
		return res;
	pos += 2;

	*pos = '\r';
	pos++;
	
	*pos = '\n';
	
	return RESULT_OK;
}

/*****************************************************************************/

result_e modbus_msg2cmd(const uint8_t * msg, modbus_cmd_s * cmd)
{
	uint16_t		msg_length;
	uint8_t			tmp;
	result_e		res;
	uint8_t			crc[2];
		
	memset(cmd, 0x00, sizeof(modbus_cmd_s));
	
	for (msg_length = 0; msg_length < MODBUS_MAX_MSG_LENGTH; msg_length++)
		if (msg[msg_length] == '\r' && msg[msg_length + 1] == '\n')
			break;
			
	if (msg_length == MODBUS_MAX_MSG_LENGTH)
		return RESULT_MSG_TOO_BIG;
		
	msg_length += 2;
	
	if ((res = modbus_crc(msg + 1, msg_length - 5, crc)) != RESULT_OK)
		return res;
		
	if (*crc != *(msg + msg_length - 4) || *(crc + 1) != *(msg + msg_length - 3))
		return RESULT_BAD_CRC;
		
	msg++;
	if ((res = modbus_char2byte(msg, &(cmd->device_id))) != RESULT_OK)
		return res;
		
	msg += 2;
	if ((res = modbus_char2byte(msg, (uint8_t *)(&(cmd->cmd_code)))) != RESULT_OK)
		return res;
	
	if (cmd->cmd_code == MODBUS_READ && msg_length != MODBUS_READ_REQ_LENGTH)
	{
		cmd->cmd_type = MODBUS_ACK;
		msg += 2;
		if ((res = modbus_char2byte(msg, &tmp)) != RESULT_OK)
			return res;
		tmp /= 2;
		cmd->addr = (tmp < MODBUS_MAX_WORDS_READ)?tmp:MODBUS_MAX_WORDS_READ;
		msg += 2;

		for (tmp = 0; tmp < cmd->addr; tmp++, msg += 4)
			if ((res = modbus_char2word(msg, cmd->value + tmp)) != RESULT_OK)
				return res;
	}
	else
	{
		msg += 2;
		cmd->cmd_type = MODBUS_REQ;
		if ((res = modbus_char2word(msg, &(cmd->addr))) != RESULT_OK)
			return res;
		msg += 4;
		if ((res = modbus_char2word(msg, cmd->value)) != RESULT_OK)
			return res;
	}

	return RESULT_OK;
}
