#!/usr/bin/awk -f
{
	if ($1 == "#")
	{
		printf("\n<hr>\n\n### %s\n\n", $2) 
		print("|OK|Ref|Value|X|Y|")
		print("|---:|---|-----|-|-|")
	}
	else
	{
		printf("|<ul><li> [ ] </li></ul>|%s|%s|%.2f|%.2f|\n", $4, $6, $7 / 10, $8 / 10)
	}
}
