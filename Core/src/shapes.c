#include "main.h"
/*
void	shape_1(t_vars *p_vars)
{
	int		x;
	int		y_dvdr;

	y_dvdr = 128;
	x = 0;
	while (x <= 255)
	{
		p_vars->sin_val2[x] = y_dvdr +
			((int)((-6) * sinf(
			x * (acos(-1.0) / 18.2857))));
		x++;
	}
	x = 0;
	while (x <= 255)
	{
		p_vars->sin_val3[x] =
			((int)((15) * sinf(
			x * (acos(-1.0) / y_dvdr))));
		x++;
	}
	x = 0;
	while (x <= 255)
	{
		p_vars->sin_val[x] = y_dvdr +
			p_vars->sin_val1[x]
			+ p_vars->sin_val2[x];
//			+ p_vars->sin_val3[x];
		x++;
	}
}
*/
//	просто синусоида
void	shape_2(t_vars *p_vars)
{
	int		x;
	int		y_dvdr, y_base;

	y_dvdr = 128;
	y_base = y_dvdr - 1;
	x = 0;
	while (x <= 255)
	{
		p_vars->sin_val1[x] = y_dvdr +
			((int)((y_base) * sinf(
			x * (acos(-1.0) / y_dvdr))));
		x++;
	}
}
/*
//	полученная через адовую формулу
//	трапеция с полкой на треть периода
//	грубая синусоида по сути (см. shape_2)
void	shape_3(t_vars *p_vars)
{
	int		x;
	int		y_dvdr, y_base;
	int		nfp1, nfp2;
	unsigned char	val;

	x = 0;
	y_dvdr = 128;
	y_base = y_dvdr - 1;
	while (x <= 255)
	{
		nfp1 = 6 * ((x - 64) * acos(-1.0) / y_dvdr) / (acos(-1.0));
		nfp2 = 0.5 * nfp1;
		val = (int)y_base
		/ (cosf(
		(2 * acos(-1.0) / 6) * (0.5 * nfp1 - nfp2)
		- ((x - 64) * ((acos(-1.0) / (y_dvdr))))
		+ ((acos(-1.0) / 6) * nfp1)));
		p_vars->sin_val1[x] = y_dvdr +
			((int)(val * sinf(
			x * (acos(-1.0) / y_dvdr))));
		x++;
	}
}
*/
//	трапеция с полкой на треть периода
//	грубая синусоида по сути (см. shape_2)
void	shape_4(t_vars *p_vars)
{
	int		x;
	int		y_dvdr;

	x = 0;
	y_dvdr = 128;
	while (x <= 42)
	{
		p_vars->sin_val1[x] =
//		(uint16_t)
		(y_dvdr + (x * 3));
		x++;
	}
	while (x <= 85)
	{
		p_vars->sin_val1[x] =
//		(uint16_t)
		((y_dvdr * 2) - 1);
		x++;
	}
	while (x <= 169)
	{
		p_vars->sin_val1[x] =
//		(uint16_t)
		((y_dvdr * 2) - 1 - (x * 3));
		x++;
	}
	while (x <= 213)
	{
		p_vars->sin_val1[x] =
//		(uint16_t)
		1;
		x++;
	}
	while (x <= 255)
	{
		p_vars->sin_val1[x] =
//		(uint16_t)
		(y_dvdr + (x * 3));
		x++;
	}
}

//	трапеция с полкой на треть периода
//	грубое "седло" по сути (см. ниже)
void	shape_5(t_vars *p_vars)
{
	int		x;
	int		y_dvdr;

	x = 0;
	y_dvdr = 128;
	while (x <= 85)
	{
		p_vars->sin_val1[x] = (y_dvdr * 2) - 1;
		x++;
	}
	while (x <= 127)
	{
		p_vars->sin_val1[x] = (y_dvdr * 2) - 1 - (x * 6);
		x++;
	}
	while (x <= 213)
	{
		p_vars->sin_val1[x] = 1;
		x++;
	}
	while (x <= 255)
	{
		p_vars->sin_val1[x] = 1 + (x * 6);
		x++;
	}
}

//	седловидная функция, полученная суммой синусоиды
//	и пилообразной функции (какой блин амплитуды!?)
void	shape_6(t_vars *p_vars)
{
	int		x;
	int		y_dvdr, y_base;

	y_dvdr = 128;
	y_base = y_dvdr + 18;
	x = 0;
	while (x <= 255)
	{
		p_vars->sin_val0[x] = y_dvdr +
			((int)((y_base) * sinf(
			x * (acos(-1.0) / y_dvdr))));
		x++;
	}
	x = 0;
	while (x <= 21)
	{
		p_vars->sin_val2[x] = x;
		x++;
	}
	while (x <= 64)
	{
		p_vars->sin_val2[x] = - (x - 42);
		x++;
	}
	while (x <= 106)
	{
		p_vars->sin_val2[x] = x - 86;
		x++;
	}
	while (x <= 148)
	{
		p_vars->sin_val2[x] = - (x - 127);
		x++;
	}
	while (x <= 192)
	{
		p_vars->sin_val2[x] = x - 171;
		x++;
	}
	while (x <= 234)
	{
		p_vars->sin_val2[x] = - (x - 213);
		x++;
	}
	while (x <= 255)
	{
		p_vars->sin_val2[x] = x - 255;
		x++;
	}

	x = 0;
	while (x <= 255)
	{
		p_vars->sin_val1[x] =
		p_vars->sin_val0[x]
		+ ((p_vars->sin_val2[x] + 1) * 1.5);
		x++;
	}
}

