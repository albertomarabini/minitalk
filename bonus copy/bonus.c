/**
 * Determines the number of bytes in a UTF-8 character based on its first byte.
 * The length of an UTF-8 character is determined by the the bits in the first
 * byte following this schema:
 * U+0000   U+007F   -> 0xxxxxxx                            -> 0-127
 *                mask: 10000000 (or 0x80 in hex) -> 0x80 & c == 0
 * U+0080   U+07FF   -> 110xxxxx 10xxxxxx                   -> 128-2047
 *                mask: 11100000 (or 0xE0 in hex) -> 0xE0 & c == 0xC0
 * U+0800   U+FFFF   -> 1110xxxx 10xxxxxx 10xxxxxx          -> 2048-65535
 *                mask: 11110000 (or 0xF0 in hex) -> 0xF0 & c == 0xE0
 * U+10000  U+10FFFF -> 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx -> 65536-1114111
 *                mask: 11111000 (or 0xF8 in hex) -> 0xF8 & c == 0xF0
 *
 * @param c The first byte of the UTF-8 character.
 * @return The total number of bytes the UTF-8 character.
 *         Returns -1 if c is a null byte ('\0').
 *         Returns 0 if c does not fit any of the UTF-8 encoding schemas.
 */


wchar_t	utf8_to_wchar(const unsigned char *utf8_seq)
{
	size_t	len;
	wchar_t	wc;

	len = utf8_char_len(utf8_seq[0]);
	wc = 0;
	if (len == 1)
	{
		wc = utf8_seq[0];
	}
	else if (len == 2)
	{
		wc = (utf8_seq[0] & 0x1F) << 6;
		wc |= (utf8_seq[1] & 0x3F);
	}
	else if (len == 3)
	{
		wc = (utf8_seq[0] & 0x0F) << 12;
		wc |= (utf8_seq[1] & 0x3F) << 6;
		wc |= (utf8_seq[2] & 0x3F);
	}
	else if (len == 4)
	{
		wc = (utf8_seq[0] & 0x07) << 18;
		wc |= (utf8_seq[1] & 0x3F) << 12;
		wc |= (utf8_seq[2] & 0x3F) << 6;
		wc |= (utf8_seq[3] & 0x3F);
	}
	return (wc);
}

int	main(void)
{
	wchar_t			wc;
	unsigned char	utf8_seq[];

	utf8_seq[] = {0xE2, 0x82, 0xAC};
	// UTF-8 sequence for the Euro sign (€)
	wc = utf8_to_wchar(utf8_seq);
	wprintf(L"The wide character is: %lc (0x%lx)\n", wc, wc);
	return (0);
}
