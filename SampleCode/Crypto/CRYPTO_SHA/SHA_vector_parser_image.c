
extern uint32_t  VectorDataBase, VectorDataLimit;

static uint8_t *file_base;
static uint32_t file_idx, file_size;

static char  g_line[0x10000];

int  open_test_file(char *filename)
{
	file_base = (uint8_t *)&VectorDataBase;
	file_size = ptr_to_u32(&VectorDataLimit) - ptr_to_u32(&VectorDataBase);
	file_idx = 0;
	return 1;
}

int  close_test_file()
{
	return 0;
}

static int  read_file(uint8_t *buffer, int length)
{
	if (file_idx+1 >= file_size)
		return -1;
	memcpy(buffer, &file_base[file_idx], length);
	file_idx += length;
	return 0;
}

int  get_line(void)
{
	int         i;
	uint8_t     ch;

	if (file_idx+1 >= file_size)
	{
		//sysprintf("EOF.\n");
		return -1;
	}

	//memset(g_line, 0, sizeof(g_line));

	for (i = 0;  ; i++)
	{
		g_line[i] = 0;

		if (read_file(&ch, 1) < 0)
			return 0;

		if ((ch == 0x0D) || (ch == 0x0A))
			break;

		g_line[i] = ch;
	}

	while (1)
	{
		if (read_file(&ch, 1) < 0)
			return 0;

		if ((ch != 0x0D) && (ch != 0x0A))
			break;
	}
	file_idx--;
	return 0;
}

int  is_hex_char(char c)
{
	if ((c >= '0') && (c <= '9'))
		return 1;
	if ((c >= 'a') && (c <= 'f'))
		return 1;
	if ((c >= 'A') && (c <= 'F'))
		return 1;
	return 0;
}

uint8_t  char_to_hex(uint8_t c)
{
	if ((c >= '0') && (c <= '9'))
		return c - '0';
	if ((c >= 'a') && (c <= 'f'))
		return c - 'a' + 10;
	if ((c >= 'A') && (c <= 'F'))
		return c - 'A' + 10;
	return 0;
}

int  str_to_hex(uint8_t *str, uint8_t *hex, int swap)
{
	int         i, count = 0, actual_len;
	uint8_t     val8;

	while (*str)
	{
		if (!is_hex_char(*str))
		{
			//sysprintf("ERROR - not hex!!\n");
			return count;
		}

		val8 = char_to_hex(*str);
		str++;

		if (!is_hex_char(*str))
		{
			//sysprintf("ERROR - not hex!!\n");
			return count;
		}

		val8 = (val8 << 4) | char_to_hex(*str);
		str++;

		hex[count] = val8;
		//sysprintf("hex = 0x%x\n", val8);
		count++;
	}

	actual_len = count;

	for ( ; count % 4 ; count++)
		hex[count] = 0;

	if (!swap)
		return actual_len;

	// SWAP
	for (i = 0; i < count; i+=4)
	{
		val8 = hex[i];
		hex[i] = hex[i+3];
		hex[i+3] = val8;

		val8 = hex[i+1];
		hex[i+1] = hex[i+2];
		hex[i+2] = val8;
	}

	return actual_len;
}

int  str_to_decimal(uint8_t *str)
{
	int         val32;
	uint8_t     val8;

	val32 = 0;
	while (*str)
	{
		if ((*str < '0') || (*str > '9'))
		{
			return val32;
		}
		val32 = (val32 * 10) + (*str - '0');
		str++;
	}
	return val32;
}

static char * find_alphanumeric(char *p)
{
	for (; *p != '\0'; p++)
	{
		if (((*p >= 'A') && (*p <= 'Z')) || ((*p >= 'a') && (*p <= 'z')) ||
			((*p >= '0') && (*p <= '9')))
			return p;
	}
	return NULL;
}

int  get_next_pattern(void)
{
	int     line_num = 1;
	char    *p;

	while (get_line() == 0)
	{
		// sysprintf("LINE %d = %s\n", line_num, g_line);
		line_num++;

		if (g_line[0] == '#')
			continue;

		p = find_alphanumeric(g_line);

		if ((strncmp(p ,"L =", 3) == 0) || (strncmp(p ,"L=", 2) == 0))
		{
			p++;
			p = find_alphanumeric(p);
			g_digest_len = str_to_decimal(p);
			continue;
		}

		if (strncmp(p, "Len", 3) == 0)
		{
			p += 3;
			p = find_alphanumeric(p);
			g_msg_bit_len = str_to_decimal(p);
			continue;
		}

		if (strncmp(p, "Msg", 3) == 0)
		{
			p += 3;
			p = find_alphanumeric(p);
			str_to_hex(p, &g_sha_msg[0], 0);
			continue;
		}

		if (strncmp(p, "MD", 2) == 0)
		{
			p += 2;
			p = find_alphanumeric(p);
			str_to_hex(p, &g_sha_digest[0], 1);
			return 0;
		}
	}
	return -1;
}
