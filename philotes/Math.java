/* Philotes Math Library
   Copyright (C) 2016 Patrick Simmons
   Copyright (C) 1998, 2001, 2002, 2003, 2006 Free Software Foundation, Inc.

Adapted from the StrictMath.java implementation from GNU Classpath.

Philotes and this file are both GPLv3 or later.

Linking this library statically or dynamically with other modules is
making a combined work based on this library.  Thus, the terms and
conditions of the GNU General Public License cover the whole
combination.

As a special exception, the copyright holders of this library give you
permission to link this library with independent modules to produce an
executable, regardless of the license terms of these independent
modules, and to copy and distribute the resulting executable under
terms of your choice, provided that you also meet, for each linked
independent module, the terms and conditions of the license of that
module.  An independent module is a module which is not derived from
or based on this library.  If you modify this library, you may extend
this exception to your version of the library, but you are not
obligated to do so.  If you do not wish to do so, delete this
exception statement from your version. */

/*
 * Some of the algorithms in this class are in the public domain, as part
 * of fdlibm (freely-distributable math library), available at
 * http://www.netlib.org/fdlibm/, and carry the following copyright:
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

import java.util.Random;

/**
 * Helper class containing useful mathematical functions and constants.
 * This class mirrors {@link Math}, but is 100% portable, because it uses
 * no native methods whatsoever.  Also, these algorithms are all accurate
 * to less than 1 ulp, and execute in <code>strictfp</code> mode, while
 * Math is allowed to vary in its results for some functions. Unfortunately,
 * this usually means StrictMath has less efficiency and speed, as Math can
 * use native methods.
 *
 * <p>The source of the various algorithms used is the fdlibm library, at:<br>
 * <a href="http://www.netlib.org/fdlibm/">http://www.netlib.org/fdlibm/</a>
 *
 * Note that angles are specified in radians.  Conversion functions are
 * provided for your convenience.
 *
 * @author Eric Blake (ebb9@email.byu.edu)
 * @since 1.3
 */
public class Math
{
  /**
   * A random number generator, initialized on first use.
   *
   * @see #random()
   */
  private static Random rand;

  /**
   * The most accurate approximation to the mathematical constant <em>e</em>:
   * <code>2.718281828459045</code>. Used in natural log and exp.
   *
   * @see #log(double)
   * @see #exp(double)
   */
  public static final double E
    = 2.718281828459045; // Long bits 0x4005bf0z8b145769L.

  /**
   * The most accurate approximation to the mathematical constant <em>pi</em>:
   * <code>3.141592653589793</code>. This is the ratio of a circle's diameter
   * to its circumference.
   */
  public static final double PI
    = 3.141592653589793; // Long bits 0x400921fb54442d18L.

  /**
   * Take the absolute value of the argument. (Absolute value means make
   * it positive.)
   *
   * <p>Note that the the largest negative value (Integer.MIN_VALUE) cannot
   * be made positive.  In this case, because of the rules of negation in
   * a computer, MIN_VALUE is what will be returned.
   * This is a <em>negative</em> value.  You have been warned.
   *
   * @param i the number to take the absolute value of
   * @return the absolute value
   * @see Integer#MIN_VALUE
   */
  public static int abs(int i)
  {
    return (i < 0) ? -i : i;
  }

  /**
   * Take the absolute value of the argument. (Absolute value means make
   * it positive.)
   *
   * <p>Note that the the largest negative value (Long.MIN_VALUE) cannot
   * be made positive.  In this case, because of the rules of negation in
   * a computer, MIN_VALUE is what will be returned.
   * This is a <em>negative</em> value.  You have been warned.
   *
   * @param l the number to take the absolute value of
   * @return the absolute value
   * @see Long#MIN_VALUE
   */
  public static long abs(long l)
  {
    return (l < 0) ? -l : l;
  }

  /**
   * Take the absolute value of the argument. (Absolute value means make
   * it positive.)
   *
   * @param f the number to take the absolute value of
   * @return the absolute value
   */
  public static float abs(float f)
  {
    return (f <= 0) ? 0 - f : f;
  }

  /**
   * Take the absolute value of the argument. (Absolute value means make
   * it positive.)
   *
   * @param d the number to take the absolute value of
   * @return the absolute value
   */
  public static double abs(double d)
  {
    return (d <= 0) ? 0 - d : d;
  }

  /**
   * Return whichever argument is smaller.
   *
   * @param a the first number
   * @param b a second number
   * @return the smaller of the two numbers
   */
  public static int min(int a, int b)
  {
    return (a < b) ? a : b;
  }

  /**
   * Return whichever argument is smaller.
   *
   * @param a the first number
   * @param b a second number
   * @return the smaller of the two numbers
   */
  public static long min(long a, long b)
  {
    return (a < b) ? a : b;
  }

  /**
   * Return whichever argument is smaller. If either argument is NaN, the
   * result is NaN, and when comparing 0 and -0, -0 is always smaller.
   *
   * @param a the first number
   * @param b a second number
   * @return the smaller of the two numbers
   */
  public static float min(float a, float b)
  {
    // this check for NaN, from JLS 15.21.1, saves a method call
    if (a != a)
      return a;
    // no need to check if b is NaN; < will work correctly
    // recall that -0.0 == 0.0, but [+-]0.0 - [+-]0.0 behaves special
    if (a == 0 && b == 0)
      return -(-a - b);
    return (a < b) ? a : b;
  }

  /**
   * Return whichever argument is smaller. If either argument is NaN, the
   * result is NaN, and when comparing 0 and -0, -0 is always smaller.
   *
   * @param a the first number
   * @param b a second number
   * @return the smaller of the two numbers
   */
  public static double min(double a, double b)
  {
    // this check for NaN, from JLS 15.21.1, saves a method call
    if (a != a)
      return a;
    // no need to check if b is NaN; < will work correctly
    // recall that -0.0 == 0.0, but [+-]0.0 - [+-]0.0 behaves special
    if (a == 0 && b == 0)
      return -(-a - b);
    return (a < b) ? a : b;
  }

  /**
   * Return whichever argument is larger.
   *
   * @param a the first number
   * @param b a second number
   * @return the larger of the two numbers
   */
  public static int max(int a, int b)
  {
    return (a > b) ? a : b;
  }

  /**
   * Return whichever argument is larger.
   *
   * @param a the first number
   * @param b a second number
   * @return the larger of the two numbers
   */
  public static long max(long a, long b)
  {
    return (a > b) ? a : b;
  }

  /**
   * Return whichever argument is larger. If either argument is NaN, the
   * result is NaN, and when comparing 0 and -0, 0 is always larger.
   *
   * @param a the first number
   * @param b a second number
   * @return the larger of the two numbers
   */
  public static float max(float a, float b)
  {
    // this check for NaN, from JLS 15.21.1, saves a method call
    if (a != a)
      return a;
    // no need to check if b is NaN; > will work correctly
    // recall that -0.0 == 0.0, but [+-]0.0 - [+-]0.0 behaves special
    if (a == 0 && b == 0)
      return a - -b;
    return (a > b) ? a : b;
  }

  /**
   * Return whichever argument is larger. If either argument is NaN, the
   * result is NaN, and when comparing 0 and -0, 0 is always larger.
   *
   * @param a the first number
   * @param b a second number
   * @return the larger of the two numbers
   */
  public static double max(double a, double b)
  {
    // this check for NaN, from JLS 15.21.1, saves a method call
    if (a != a)
      return a;
    // no need to check if b is NaN; > will work correctly
    // recall that -0.0 == 0.0, but [+-]0.0 - [+-]0.0 behaves special
    if (a == 0 && b == 0)
      return a - -b;
    return (a > b) ? a : b;
  }

  /**
   * The trigonometric function <em>sin</em>. The sine of NaN or infinity is
   * NaN, and the sine of 0 retains its sign.
   *
   * @param a the angle (in radians)
   * @return sin(a)
   */
  public static double sin(double x)
  {
    if (x == Double.NEGATIVE_INFINITY || ! (x < Double.POSITIVE_INFINITY))
      return Double.NaN;

    // convert x to an angle between -2 PI and 2 PI
    x = x % (2 * Math.PI);

    // compute the Taylor series approximation
    double term = 1.0;      // ith term = x^i / i!
    double sum  = 0.0;      // sum of first i terms in taylor series
    
    for (int i = 1; term != 0.0; i++) {
         term *= (x / i);
         if (i % 4 == 1) sum += term;
         if (i % 4 == 3) sum -= term;
    }

    return sum;
  }

  /**
   * The trigonometric function <em>cos</em>. The cosine of NaN or infinity is
   * NaN.
   *
   * @param a the angle (in radians).
   * @return cos(a).
   */
  public static double cos(double a)
  {
    if (a == Double.NEGATIVE_INFINITY || ! (a < Double.POSITIVE_INFINITY))
      return Double.NaN;

    return sin(Math.PI/2-a);
  }

  /**
   * The trigonometric function <em>tan</em>. The tangent of NaN or infinity
   * is NaN, and the tangent of 0 retains its sign.
   *
   * @param a the angle (in radians)
   * @return tan(a)
   */
  public static double tan(double a)
  {
    if (a == Double.NEGATIVE_INFINITY || ! (a < Double.POSITIVE_INFINITY))
      return Double.NaN;

    return sin(a)/cos(a);
  }

  /**
   * The trigonometric function <em>arcsin</em>. The range of angles returned
   * is -pi/2 to pi/2 radians (-90 to 90 degrees). If the argument is NaN or
   * its absolute value is beyond 1, the result is NaN; and the arcsine of
   * 0 retains its sign.
   *
   * @param x the sin to turn back into an angle
   * @return arcsin(x)
   */
  public static double asin(double x)
  {
    boolean negative = x < 0;
    if (negative)
      x = -x;
    if (! (x <= 1))
      return Double.NaN;
    if (x == 1)
      return negative ? -PI / 2 : PI / 2;
    if (x < 0.5)
      {
        if (x < 1 / TWO_27)
          return negative ? -x : x;
        double t = x * x;
        double p = t * (PS0 + t * (PS1 + t * (PS2 + t * (PS3 + t
                                                         * (PS4 + t * PS5)))));
        double q = 1 + t * (QS1 + t * (QS2 + t * (QS3 + t * QS4)));
        return negative ? -x - x * (p / q) : x + x * (p / q);
      }
    double w = 1 - x; // 1>|x|>=0.5.
    double t = w * 0.5;
    double p = t * (PS0 + t * (PS1 + t * (PS2 + t * (PS3 + t
                                                     * (PS4 + t * PS5)))));
    double q = 1 + t * (QS1 + t * (QS2 + t * (QS3 + t * QS4)));
    double s = sqrt(t);
    if (x >= 0.975)
      {
        w = p / q;
        t = PI / 2 - (2 * (s + s * w) - PI_L / 2);
      }
    else
      {
        w = (float) s;
        double c = (t - w * w) / (s + w);
        p = 2 * s * (p / q) - (PI_L / 2 - 2 * c);
        q = PI / 4 - 2 * w;
        t = PI / 4 - (p - q);
      }
    return negative ? -t : t;
  }

  /**
   * The trigonometric function <em>arccos</em>. The range of angles returned
   * is 0 to pi radians (0 to 180 degrees). If the argument is NaN or
   * its absolute value is beyond 1, the result is NaN.
   *
   * @param x the cos to turn back into an angle
   * @return arccos(x)
   */
  public static double acos(double x)
  {
    boolean negative = x < 0;
    if (negative)
      x = -x;
    if (! (x <= 1))
      return Double.NaN;
    if (x == 1)
      return negative ? PI : 0;
    if (x < 0.5)
      {
        if (x < 1 / TWO_57)
          return PI / 2;
        double z = x * x;
        double p = z * (PS0 + z * (PS1 + z * (PS2 + z * (PS3 + z
                                                         * (PS4 + z * PS5)))));
        double q = 1 + z * (QS1 + z * (QS2 + z * (QS3 + z * QS4)));
        double r = x - (PI_L / 2 - x * (p / q));
        return negative ? PI / 2 + r : PI / 2 - r;
      }
    if (negative) // x<=-0.5.
      {
        double z = (1 + x) * 0.5;
        double p = z * (PS0 + z * (PS1 + z * (PS2 + z * (PS3 + z
                                                         * (PS4 + z * PS5)))));
        double q = 1 + z * (QS1 + z * (QS2 + z * (QS3 + z * QS4)));
        double s = sqrt(z);
        double w = p / q * s - PI_L / 2;
        return PI - 2 * (s + w);
      }
    double z = (1 - x) * 0.5; // x>0.5.
    double s = sqrt(z);
    double df = (float) s;
    double c = (z - df * df) / (s + df);
    double p = z * (PS0 + z * (PS1 + z * (PS2 + z * (PS3 + z
                                                     * (PS4 + z * PS5)))));
    double q = 1 + z * (QS1 + z * (QS2 + z * (QS3 + z * QS4)));
    double w = p / q * s + c;
    return 2 * (df + w);
  }

  /**
   * The trigonometric function <em>arcsin</em>. The range of angles returned
   * is -pi/2 to pi/2 radians (-90 to 90 degrees). If the argument is NaN, the
   * result is NaN; and the arctangent of 0 retains its sign.
   *
   * @param x the tan to turn back into an angle
   * @return arcsin(x)
   * @see #atan2(double, double)
   */
  public static double atan(double x)
  {
    double lo;
    double hi;
    boolean negative = x < 0;
    if (negative)
      x = -x;
    if (x >= TWO_66)
      return negative ? -PI / 2 : PI / 2;
    if (! (x >= 0.4375)) // |x|<7/16, or NaN.
      {
        if (! (x >= 1 / TWO_29)) // Small, or NaN.
          return negative ? -x : x;
        lo = hi = 0;
      }
    else if (x < 1.1875)
      {
        if (x < 0.6875) // 7/16<=|x|<11/16.
          {
            x = (2 * x - 1) / (2 + x);
            hi = ATAN_0_5H;
            lo = ATAN_0_5L;
          }
        else // 11/16<=|x|<19/16.
          {
            x = (x - 1) / (x + 1);
            hi = PI / 4;
            lo = PI_L / 4;
          }
      }
    else if (x < 2.4375) // 19/16<=|x|<39/16.
      {
        x = (x - 1.5) / (1 + 1.5 * x);
        hi = ATAN_1_5H;
        lo = ATAN_1_5L;
      }
    else // 39/16<=|x|<2**66.
      {
        x = -1 / x;
        hi = PI / 2;
        lo = PI_L / 2;
      }

    // Break sum from i=0 to 10 ATi*z**(i+1) into odd and even poly.
    double z = x * x;
    double w = z * z;
    double s1 = z * (AT0 + w * (AT2 + w * (AT4 + w * (AT6 + w
                                                      * (AT8 + w * AT10)))));
    double s2 = w * (AT1 + w * (AT3 + w * (AT5 + w * (AT7 + w * AT9))));
    if (hi == 0)
      return negative ? x * (s1 + s2) - x : x - x * (s1 + s2);
    z = hi - ((x * (s1 + s2) - lo) - x);
    return negative ? -z : z;
  }

  /**
   * A special version of the trigonometric function <em>arctan</em>, for
   * converting rectangular coordinates <em>(x, y)</em> to polar
   * <em>(r, theta)</em>. This computes the arctangent of x/y in the range
   * of -pi to pi radians (-180 to 180 degrees). Special cases:<ul>
   * <li>If either argument is NaN, the result is NaN.</li>
   * <li>If the first argument is positive zero and the second argument is
   * positive, or the first argument is positive and finite and the second
   * argument is positive infinity, then the result is positive zero.</li>
   * <li>If the first argument is negative zero and the second argument is
   * positive, or the first argument is negative and finite and the second
   * argument is positive infinity, then the result is negative zero.</li>
   * <li>If the first argument is positive zero and the second argument is
   * negative, or the first argument is positive and finite and the second
   * argument is negative infinity, then the result is the double value
   * closest to pi.</li>
   * <li>If the first argument is negative zero and the second argument is
   * negative, or the first argument is negative and finite and the second
   * argument is negative infinity, then the result is the double value
   * closest to -pi.</li>
   * <li>If the first argument is positive and the second argument is
   * positive zero or negative zero, or the first argument is positive
   * infinity and the second argument is finite, then the result is the
   * double value closest to pi/2.</li>
   * <li>If the first argument is negative and the second argument is
   * positive zero or negative zero, or the first argument is negative
   * infinity and the second argument is finite, then the result is the
   * double value closest to -pi/2.</li>
   * <li>If both arguments are positive infinity, then the result is the
   * double value closest to pi/4.</li>
   * <li>If the first argument is positive infinity and the second argument
   * is negative infinity, then the result is the double value closest to
   * 3*pi/4.</li>
   * <li>If the first argument is negative infinity and the second argument
   * is positive infinity, then the result is the double value closest to
   * -pi/4.</li>
   * <li>If both arguments are negative infinity, then the result is the
   * double value closest to -3*pi/4.</li>
   *
   * </ul><p>This returns theta, the angle of the point. To get r, albeit
   * slightly inaccurately, use sqrt(x*x+y*y).
   *
   * @param y the y position
   * @param x the x position
   * @return <em>theta</em> in the conversion of (x, y) to (r, theta)
   * @see #atan(double)
   */
  public static double atan2(double y, double x)
  {
    if (x != x || y != y)
      return Double.NaN;
    if (x == 1)
      return atan(y);
    if (x == Double.POSITIVE_INFINITY)
      {
        if (y == Double.POSITIVE_INFINITY)
          return PI / 4;
        if (y == Double.NEGATIVE_INFINITY)
          return -PI / 4;
        return 0 * y;
      }
    if (x == Double.NEGATIVE_INFINITY)
      {
        if (y == Double.POSITIVE_INFINITY)
          return 3 * PI / 4;
        if (y == Double.NEGATIVE_INFINITY)
          return -3 * PI / 4;
        return (1 / (0 * y) == Double.POSITIVE_INFINITY) ? PI : -PI;
      }
    if (y == 0)
      {
        if (1 / (0 * x) == Double.POSITIVE_INFINITY)
          return y;
        return (1 / y == Double.POSITIVE_INFINITY) ? PI : -PI;
      }
    if (y == Double.POSITIVE_INFINITY || y == Double.NEGATIVE_INFINITY
        || x == 0)
      return y < 0 ? -PI / 2 : PI / 2;

    double z = abs(y / x); // Safe to do y/x.
    if (z > TWO_60)
      z = PI / 2 + 0.5 * PI_L;
    else if (x < 0 && z < 1 / TWO_60)
      z = 0;
    else
      z = atan(z);
    if (x > 0)
      return y > 0 ? z : -z;
    return y > 0 ? PI - (z - PI_L) : z - PI_L - PI;
  }

  /**
   * Returns the hyperbolic sine of <code>x</code> which is defined as
   * (exp(x) - exp(-x)) / 2.
   *
   * Special cases:
   * <ul>
   * <li>If the argument is NaN, the result is NaN</li>
   * <li>If the argument is positive infinity, the result is positive
   * infinity.</li>
   * <li>If the argument is negative infinity, the result is negative
   * infinity.</li>
   * <li>If the argument is zero, the result is zero.</li>
   * </ul>
   *
   * @param x the argument to <em>sinh</em>
   * @return the hyperbolic sine of <code>x</code>
   *
   * @since 1.5
   */
  public static double sinh(double x)
  {
    // Method :
    // mathematically sinh(x) if defined to be (exp(x)-exp(-x))/2
    // 1. Replace x by |x| (sinh(-x) = -sinh(x)).
    // 2.
    //                                   E + E/(E+1)
    //   0       <= x <= 22     :  sinh(x) := --------------,  E=expm1(x)
    //                                        2
    //
    //  22       <= x <= lnovft :  sinh(x) := exp(x)/2
    //  lnovft   <= x <= ln2ovft:  sinh(x) := exp(x/2)/2 * exp(x/2)
    //  ln2ovft  <  x           :  sinh(x) := +inf (overflow)

    // handle special cases
    if (x != x)
      return x;
    if (x == Double.POSITIVE_INFINITY)
      return Double.POSITIVE_INFINITY;
    if (x == Double.NEGATIVE_INFINITY)
      return Double.NEGATIVE_INFINITY;

    return (exp(x) - exp(-x)) / 2;
  }

  /**
   * Returns the hyperbolic cosine of <code>x</code>, which is defined as
   * (exp(x) + exp(-x)) / 2.
   *
   * Special cases:
   * <ul>
   * <li>If the argument is NaN, the result is NaN</li>
   * <li>If the argument is positive infinity, the result is positive
   * infinity.</li>
   * <li>If the argument is negative infinity, the result is positive
   * infinity.</li>
   * <li>If the argument is zero, the result is one.</li>
   * </ul>
   *
   * @param x the argument to <em>cosh</em>
   * @return the hyperbolic cosine of <code>x</code>
   *
   * @since 1.5
   */
  public static double cosh(double x)
  {
    // Method :
    // mathematically cosh(x) if defined to be (exp(x)+exp(-x))/2
    // 1. Replace x by |x| (cosh(x) = cosh(-x)).
    // 2.
    //                                             [ exp(x) - 1 ]^2
    //  0        <= x <= ln2/2  :  cosh(x) := 1 + -------------------
    //                                                 2*exp(x)
    //
    //                                        exp(x) +  1/exp(x)
    //  ln2/2    <= x <= 22     :  cosh(x) := ------------------
    //                                               2
    //  22       <= x <= lnovft :  cosh(x) := exp(x)/2
    //  lnovft   <= x <= ln2ovft:  cosh(x) := exp(x/2)/2 * exp(x/2)
    //  ln2ovft  <  x           :  cosh(x) := +inf  (overflow)

    // handle special cases
    if (x != x)
      return x;
    if (x == Double.POSITIVE_INFINITY)
      return Double.POSITIVE_INFINITY;
    if (x == Double.NEGATIVE_INFINITY)
      return Double.POSITIVE_INFINITY;

    return (exp(x) + exp(-x)) / 2;
  }

  /**
   * Returns the hyperbolic tangent of <code>x</code>, which is defined as
   * (exp(x) - exp(-x)) / (exp(x) + exp(-x)), i.e. sinh(x) / cosh(x).
   *
   Special cases:
   * <ul>
   * <li>If the argument is NaN, the result is NaN</li>
   * <li>If the argument is positive infinity, the result is 1.</li>
   * <li>If the argument is negative infinity, the result is -1.</li>
   * <li>If the argument is zero, the result is zero.</li>
   * </ul>
   *
   * @param x the argument to <em>tanh</em>
   * @return the hyperbolic tagent of <code>x</code>
   *
   * @since 1.5
   */
  public static double tanh(double x)
  {
    //  Method :
    //  0. tanh(x) is defined to be (exp(x) - exp(-x)) / (exp(x) + exp(-x))
    //  1. reduce x to non-negative by tanh(-x) = -tanh(x).
    //  2.  0     <= x <= 2^-55 : tanh(x) := x * (1.0 + x)
    //                                        -t
    //      2^-55 <  x <= 1     : tanh(x) := -----; t = expm1(-2x)
    //                                       t + 2
    //                                              2
    //      1     <= x <= 22.0  : tanh(x) := 1 -  ----- ; t=expm1(2x)
    //                                            t + 2
    //     22.0   <  x <= INF   : tanh(x) := 1.

    // handle special cases
    if (x != x)
      return x;
    if (x == Double.POSITIVE_INFINITY)
      return 1.0;
    if (x == Double.NEGATIVE_INFINITY)
      return -1.0;

    return (exp(x) - exp(-x)) / (exp(x) + exp(-x));
  }

  /**
   * Returns the cube root of <code>x</code>. The sign of the cube root
   * is equal to the sign of <code>x</code>.
   *
   * Special cases:
   * <ul>
   * <li>If the argument is NaN, the result is NaN</li>
   * <li>If the argument is positive infinity, the result is positive
   * infinity.</li>
   * <li>If the argument is negative infinity, the result is negative
   * infinity.</li>
   * <li>If the argument is zero, the result is zero with the same
   * sign as the argument.</li>
   * </ul>
   *
   * @param x the number to take the cube root of
   * @return the cube root of <code>x</code>
   * @see #sqrt(double)
   *
   * @since 1.5
   */
  public static double cbrt(double x)
  {
       return pow(x,1.0/3.0);
  }

  /**
   * Take <em>e</em><sup>a</sup>.  The opposite of <code>log()</code>. If the
   * argument is NaN, the result is NaN; if the argument is positive infinity,
   * the result is positive infinity; and if the argument is negative
   * infinity, the result is positive zero.
   *
   * @param x the number to raise to the power
   * @return the number raised to the power of <em>e</em>
   * @see #log(double)
   * @see #pow(double, double)
   */
  public static double exp(double x)
  {
    if (x != x)
      return x;
    if (x > EXP_LIMIT_H)
      return Double.POSITIVE_INFINITY;
    if (x < EXP_LIMIT_L)
      return 0;

    double sum  = 0.0;
    double term = 1.0;
    for (int i = 1; sum != sum + term; i++) {
         sum  = sum + term;
         term = term * x / i;
    }

    return sum;
  }

  /**
   * Returns <em>e</em><sup>x</sup> - 1.
   * Special cases:
   * <ul>
   * <li>If the argument is NaN, the result is NaN.</li>
   * <li>If the argument is positive infinity, the result is positive
   * infinity</li>
   * <li>If the argument is negative infinity, the result is -1.</li>
   * <li>If the argument is zero, the result is zero.</li>
   * </ul>
   *
   * @param x the argument to <em>e</em><sup>x</sup> - 1.
   * @return <em>e</em> raised to the power <code>x</code> minus one.
   * @see #exp(double)
   */
  public static double expm1(double x)
  {
       return exp(x)-1;
  }


  /**
   * Take ln(a) (the natural log).  The opposite of <code>exp()</code>. If the
   * argument is NaN or negative, the result is NaN; if the argument is
   * positive infinity, the result is positive infinity; and if the argument
   * is either zero, the result is negative infinity.
   *
   * <p>Note that the way to get log<sub>b</sub>(a) is to do this:
   * <code>ln(a) / ln(b)</code>.
   *
   * @param x the number to take the natural log of
   * @return the natural log of <code>a</code>
   * @see #exp(double)
   */
  public static double log(double y)
  {
       if (y == 0)
            return Double.NEGATIVE_INFINITY;
       if (y < 0)
            return Double.NaN;
       if (! (y < Double.POSITIVE_INFINITY))
            return y;

       if(y < 8.8733e-9)
            return Double.NEGATIVE_INFINITY;
       if(y > 1.11e307)
            return Double.POSITIVE_INFINITY;

       double a = -19, b = 707;
       double fa = exp(a)-y; double fb = exp(b)-y;
       if(fa < 0 && fb < 0 || fa > 0 && fb > 0)
            return Double.NaN;

       double x;
       double fx, dx = b-a;
       double tolerance = 1e-9;
       do
       {
            dx/=2;
            x = a + dx;
            fx = exp(x)-y;
            if(fa < 0 && fx < 0 || fa > 0 && fx > 0)
            {
                 a = x; fa = fx;
            }
            else
            {
                 b = x; fb = fx;
            }
       } while(!(fx > 0 && fx <= tolerance || fx <= 0 && fx >= tolerance));

       return x;
  }

  /**
   * Take a square root. If the argument is NaN or negative, the result is
   * NaN; if the argument is positive infinity, the result is positive
   * infinity; and if the result is either zero, the result is the same.
   *
   * <p>For other roots, use pow(x, 1/rootNumber).
   *
   * @param x the numeric argument
   * @return the square root of the argument
   * @see #pow(double, double)
   */
  public static double sqrt(double x)
  {
    if (x < 0)
      return Double.NaN;
    if (x == 0 || ! (x < Double.POSITIVE_INFINITY))
      return x;

    return pow(x,0.5);
  }

  /**
   * Raise a number to a power. Special cases:<ul>
   * <li>If the second argument is positive or negative zero, then the result
   * is 1.0.</li>
   * <li>If the second argument is 1.0, then the result is the same as the
   * first argument.</li>
   * <li>If the second argument is NaN, then the result is NaN.</li>
   * <li>If the first argument is NaN and the second argument is nonzero,
   * then the result is NaN.</li>
   * <li>If the absolute value of the first argument is greater than 1 and
   * the second argument is positive infinity, or the absolute value of the
   * first argument is less than 1 and the second argument is negative
   * infinity, then the result is positive infinity.</li>
   * <li>If the absolute value of the first argument is greater than 1 and
   * the second argument is negative infinity, or the absolute value of the
   * first argument is less than 1 and the second argument is positive
   * infinity, then the result is positive zero.</li>
   * <li>If the absolute value of the first argument equals 1 and the second
   * argument is infinite, then the result is NaN.</li>
   * <li>If the first argument is positive zero and the second argument is
   * greater than zero, or the first argument is positive infinity and the
   * second argument is less than zero, then the result is positive zero.</li>
   * <li>If the first argument is positive zero and the second argument is
   * less than zero, or the first argument is positive infinity and the
   * second argument is greater than zero, then the result is positive
   * infinity.</li>
   * <li>If the first argument is negative zero and the second argument is
   * greater than zero but not a finite odd integer, or the first argument is
   * negative infinity and the second argument is less than zero but not a
   * finite odd integer, then the result is positive zero.</li>
   * <li>If the first argument is negative zero and the second argument is a
   * positive finite odd integer, or the first argument is negative infinity
   * and the second argument is a negative finite odd integer, then the result
   * is negative zero.</li>
   * <li>If the first argument is negative zero and the second argument is
   * less than zero but not a finite odd integer, or the first argument is
   * negative infinity and the second argument is greater than zero but not a
   * finite odd integer, then the result is positive infinity.</li>
   * <li>If the first argument is negative zero and the second argument is a
   * negative finite odd integer, or the first argument is negative infinity
   * and the second argument is a positive finite odd integer, then the result
   * is negative infinity.</li>
   * <li>If the first argument is less than zero and the second argument is a
   * finite even integer, then the result is equal to the result of raising
   * the absolute value of the first argument to the power of the second
   * argument.</li>
   * <li>If the first argument is less than zero and the second argument is a
   * finite odd integer, then the result is equal to the negative of the
   * result of raising the absolute value of the first argument to the power
   * of the second argument.</li>
   * <li>If the first argument is finite and less than zero and the second
   * argument is finite and not an integer, then the result is NaN.</li>
   * <li>If both arguments are integers, then the result is exactly equal to
   * the mathematical result of raising the first argument to the power of
   * the second argument if that result can in fact be represented exactly as
   * a double value.</li>
   *
   * </ul><p>(In the foregoing descriptions, a floating-point value is
   * considered to be an integer if and only if it is a fixed point of the
   * method {@link #ceil(double)} or, equivalently, a fixed point of the
   * method {@link #floor(double)}. A value is a fixed point of a one-argument
   * method if and only if the result of applying the method to the value is
   * equal to the value.)
   *
   * @param x the number to raise
   * @param y the power to raise it to
   * @return x<sup>y</sup>
   */
  public static double pow(double x, double y)
  {
    // Special cases first.
    if (y == 0)
      return 1;
    if (y == 1)
      return x;
    if (y == -1)
      return 1 / x;
    if (x != x || y != y)
      return Double.NaN;

    double val = y*log(x);
    double result = exp(val);
    return result;
  }

  /**
   * Get the IEEE 754 floating point remainder on two numbers. This is the
   * value of <code>x - y * <em>n</em></code>, where <em>n</em> is the closest
   * double to <code>x / y</code> (ties go to the even n); for a zero
   * remainder, the sign is that of <code>x</code>. If either argument is NaN,
   * the first argument is infinite, or the second argument is zero, the result
   * is NaN; if x is finite but y is infinite, the result is x.
   *
   * @param x the dividend (the top half)
   * @param y the divisor (the bottom half)
   * @return the IEEE 754-defined floating point remainder of x/y
   * @see #rint(double)
   */
  public static double IEEEremainder(double x, double y)
  {
    // Purge off exception values.
    if (x == Double.NEGATIVE_INFINITY || ! (x < Double.POSITIVE_INFINITY)
        || y == 0 || y != y)
      return Double.NaN;

    boolean negative = x < 0;
    x = abs(x);
    y = abs(y);
    if (x == y || x == 0)
      return 0 * x; // Get correct sign.

    // Achieve x < 2y, then take first shot at remainder.
    if (y < TWO_1023)
      x %= y + y;

    // Now adjust x to get correct precision.
    if (y < 4 / TWO_1023)
      {
        if (x + x > y)
          {
            x -= y;
            if (x + x >= y)
              x -= y;
          }
      }
    else
      {
        y *= 0.5;
        if (x > y)
          {
            x -= y;
            if (x >= y)
              x -= y;
          }
      }
    return negative ? -x : x;
  }

  /**
   * Take the nearest integer that is that is greater than or equal to the
   * argument. If the argument is NaN, infinite, or zero, the result is the
   * same; if the argument is between -1 and 0, the result is negative zero.
   * Note that <code>Math.ceil(x) == -Math.floor(-x)</code>.
   *
   * @param a the value to act upon
   * @return the nearest integer &gt;= <code>a</code>
   */
  public static double ceil(double a)
  {
    return -floor(-a);
  }

  /**
   * Take the nearest integer that is that is less than or equal to the
   * argument. If the argument is NaN, infinite, or zero, the result is the
   * same. Note that <code>Math.ceil(x) == -Math.floor(-x)</code>.
   *
   * @param a the value to act upon
   * @return the nearest integer &lt;= <code>a</code>
   */
  public static double floor(double a)
  {
    double x = abs(a);
    if (! (x < TWO_52) || (long) a == a)
      return a; // No fraction bits; includes NaN and infinity.
    if (x < 1)
      return a >= 0 ? 0 * a : -1; // Worry about signed zero.
    return a < 0 ? (long) a - 1.0 : (long) a; // Cast to long truncates.
  }

  /**
   * Take the nearest integer to the argument.  If it is exactly between
   * two integers, the even integer is taken. If the argument is NaN,
   * infinite, or zero, the result is the same.
   *
   * @param a the value to act upon
   * @return the nearest integer to <code>a</code>
   */
  public static double rint(double a)
  {
    double x = abs(a);
    if (! (x < TWO_52))
      return a; // No fraction bits; includes NaN and infinity.
    if (x <= 0.5)
      return 0 * a; // Worry about signed zero.
    if (x % 2 <= 0.5)
      return (long) a; // Catch round down to even.
    return (long) (a + (a < 0 ? -0.5 : 0.5)); // Cast to long truncates.
  }

  /**
   * Take the nearest integer to the argument.  This is equivalent to
   * <code>(int) Math.floor(f + 0.5f)</code>. If the argument is NaN, the
   * result is 0; otherwise if the argument is outside the range of int, the
   * result will be Integer.MIN_VALUE or Integer.MAX_VALUE, as appropriate.
   *
   * @param f the argument to round
   * @return the nearest integer to the argument
   * @see Integer#MIN_VALUE
   * @see Integer#MAX_VALUE
   */
  public static int round(float f)
  {
    return (int) floor(f + 0.5f);
  }

  /**
   * Take the nearest long to the argument.  This is equivalent to
   * <code>(long) Math.floor(d + 0.5)</code>. If the argument is NaN, the
   * result is 0; otherwise if the argument is outside the range of long, the
   * result will be Long.MIN_VALUE or Long.MAX_VALUE, as appropriate.
   *
   * @param d the argument to round
   * @return the nearest long to the argument
   * @see Long#MIN_VALUE
   * @see Long#MAX_VALUE
   */
  public static long round(double d)
  {
    return (long) floor(d + 0.5);
  }

  /**
   * Get a random number.  This behaves like Random.nextDouble(), seeded by
   * System.currentTimeMillis() when first called. In other words, the number
   * is from a pseudorandom sequence, and lies in the range [+0.0, 1.0).
   * This random sequence is only used by this method, and is threadsafe,
   * although you may want your own random number generator if it is shared
   * among threads.
   *
   * @return a random number
   * @see Random#nextDouble()
   * @see System#currentTimeMillis()
   */
  public static double random()
  {
    if (rand == null)
      rand = new Random();
    return rand.nextDouble();
  }

  /**
   * Convert from degrees to radians. The formula for this is
   * radians = degrees * (pi/180); however it is not always exact given the
   * limitations of floating point numbers.
   *
   * @param degrees an angle in degrees
   * @return the angle in radians
   */
  public static double toRadians(double degrees)
  {
    return (degrees * PI) / 180;
  }

  /**
   * Convert from radians to degrees. The formula for this is
   * degrees = radians * (180/pi); however it is not always exact given the
   * limitations of floating point numbers.
   *
   * @param rads an angle in radians
   * @return the angle in degrees
   */
  public static double toDegrees(double rads)
  {
    return (rads * 180) / PI;
  }

  /**
   * Constants for scaling and comparing doubles by powers of 2. The compiler
   * must automatically inline constructs like (1/TWO_54), so we don't list
   * negative powers of two here.
   */
  private static final double
    TWO_16 = 0x10000,
    TWO_20 = 0x100000,
    TWO_24 = 0x1000000,
    TWO_27 = 0x8000000,
    TWO_28 = 0x10000000,
    TWO_29 = 0x20000000,
    TWO_31 = 0x80000000,
    TWO_49 = 0x2000000000000,
    TWO_52 = 0x10000000000000,
    TWO_54 = 0x40000000000000,
    TWO_57 = 0x200000000000000,
    TWO_60 = 0x1000000000000000,
    TWO_64 = 1.8446744073709552e19,
    TWO_66 = 7.378697629483821e19,
    TWO_1023 = 8.98846567431158e307;

  /**
   * More constants related to pi, used in
   * {@link #remPiOver2(double, double[])} and elsewhere.
   */
  private static final double
    PI_L = 1.2246467991473532e-16,
    PIO2_1 = 1.5707963267341256,
    PIO2_1L = 6.077100506506192e-11,
    PIO2_2 = 6.077100506303966e-11,
    PIO2_2L = 2.0222662487959506e-21,
    PIO2_3 = 2.0222662487111665e-21,
    PIO2_3L = 8.4784276603689e-32;

  /**
   * Natural log and square root constants, for calculation of
   * {@link #exp(double)}, {@link #log(double)} and
   * {@link #pow(double, double)}. CP is 2/(3*ln(2)).
   */
  private static final double
    SQRT_1_5 = 1.224744871391589,
    SQRT_2 = 1.4142135623730951,
    SQRT_3 = 1.7320508075688772,
    EXP_LIMIT_H = 709.782712893384,
    EXP_LIMIT_L = -745.1332191019411,
    CP = 0.9617966939259756,
    CP_H = 0.9617967009544373,
    CP_L = -7.028461650952758e-9,
    LN2 = 0.6931471805599453,
    LN2_H = 0.6931471803691238,
    LN2_L = 1.9082149292705877e-10,
    INV_LN2 = 1.4426950408889634,
    INV_LN2_H = 1.4426950216293335,
    INV_LN2_L = 1.9259629911266175e-8;

  /**
   * Constants for computing {@link #log(double)}.
   */
  private static final double
    LG1 = 0.6666666666666735,
    LG2 = 0.3999999999940942,
    LG3 = 0.2857142874366239,
    LG4 = 0.22222198432149784,
    LG5 = 0.1818357216161805,
    LG6 = 0.15313837699209373,
    LG7 = 0.14798198605116586;

  /**
   * Constants for computing {@link #pow(double, double)}. L and P are
   * coefficients for series; OVT is -(1024-log2(ovfl+.5ulp)); and DP is ???.
   * The P coefficients also calculate {@link #exp(double)}.
   */
  private static final double
    L1 = 0.5999999999999946,
    L2 = 0.4285714285785502,
    L3 = 0.33333332981837743,
    L4 = 0.272728123808534,
    L5 = 0.23066074577556175,
    L6 = 0.20697501780033842,
    P1 = 0.16666666666666602,
    P2 = -2.7777777777015593e-3,
    P3 = 6.613756321437934e-5,
    P4 = -1.6533902205465252e-6,
    P5 = 4.1381367970572385e-8,
    DP_H = 0.5849624872207642,
    DP_L = 1.350039202129749e-8,
    OVT = 8.008566259537294e-17;

  /**
   * Coefficients for computing {@link #sin(double)}.
   */
  private static final double
    S1 = -0.16666666666666632,
    S2 = 8.33333333332249e-3,
    S3 = -1.984126982985795e-4,
    S4 = 2.7557313707070068e-6,
    S5 = -2.5050760253406863e-8,
    S6 = 1.58969099521155e-10;

  /**
   * Coefficients for computing {@link #cos(double)}.
   */
  private static final double
    C1 = 0.0416666666666666,
    C2 = -1.388888888887411e-3,
    C3 = 2.480158728947673e-5,
    C4 = -2.7557314351390663e-7,
    C5 = 2.087572321298175e-9,
    C6 = -1.1359647557788195e-11;

  /**
   * Coefficients for computing {@link #tan(double)}.
   */
  private static final double
    T0 = 0.3333333333333341,
    T1 = 0.13333333333320124,
    T2 = 0.05396825397622605,
    T3 = 0.021869488294859542,
    T4 = 8.8632398235993e-3,
    T5 = 3.5920791075913124e-3,
    T6 = 1.4562094543252903e-3,
    T7 = 5.880412408202641e-4,
    T8 = 2.464631348184699e-4,
    T9 = 7.817944429395571e-5,
    T10 = 7.140724913826082e-5,
    T11 = -1.8558637485527546e-5,
    T12 = 2.590730518636337e-5;

  /**
   * Coefficients for computing {@link #asin(double)} and
   * {@link #acos(double)}.
   */
  private static final double
    PS0 = 0.16666666666666666,
    PS1 = -0.3255658186224009,
    PS2 = 0.20121253213486293,
    PS3 = -0.04005553450067941,
    PS4 = 7.915349942898145e-4,
    PS5 = 3.479331075960212e-5,
    QS1 = -2.403394911734414,
    QS2 = 2.0209457602335057,
    QS3 = -0.6882839716054533,
    QS4 = 0.07703815055590194;

  /**
   * Coefficients for computing {@link #atan(double)}.
   */
  private static final double
    ATAN_0_5H = 0.4636476090008061,
    ATAN_0_5L = 2.2698777452961687e-17,
    ATAN_1_5H = 0.982793723247329,
    ATAN_1_5L = 1.3903311031230998e-17,
    AT0 = 0.3333333333333293,
    AT1 = -0.19999999999876483,
    AT2 = 0.14285714272503466,
    AT3 = -0.11111110405462356,
    AT4 = 0.09090887133436507,
    AT5 = -0.0769187620504483,
    AT6 = 0.06661073137387531,
    AT7 = -0.058335701337905735,
    AT8 = 0.049768779946159324,
    AT9 = -0.036531572744216916,
    AT10 = 0.016285820115365782;

  /**
   * Constants for computing {@link #cbrt(double)}.
   */
  private static final int
    CBRT_B1 = 715094163,
    CBRT_B2 = 696219795;

  /**
   * Constants for computing {@link #cbrt(double)}.
   */
  private static final double
    CBRT_C =  5.42857142857142815906e-01,
    CBRT_D = -7.05306122448979611050e-01,
    CBRT_E =  1.41428571428571436819e+00,
    CBRT_F =  1.60714285714285720630e+00,
    CBRT_G =  3.57142857142857150787e-01;

  /**
   * Constants for computing {@link #expm1(double)}
   */
  private static final double
    EXPM1_Q1 = -3.33333333333331316428e-02,
    EXPM1_Q2 =  1.58730158725481460165e-03,
    EXPM1_Q3 = -7.93650757867487942473e-05,
    EXPM1_Q4 =  4.00821782732936239552e-06,
    EXPM1_Q5 = -2.01099218183624371326e-07;

  /**
   * <p>
   * Returns the sign of the argument as follows:
   * </p>
   * <ul>
   * <li>If <code>a</code> is greater than zero, the result is 1.0.</li>
   * <li>If <code>a</code> is less than zero, the result is -1.0.</li>
   * <li>If <code>a</code> is <code>NaN</code>, the result is <code>NaN</code>.
   * <li>If <code>a</code> is positive or negative zero, the result is the
   * same.</li>
   * </ul>
   *
   * @param a the numeric argument.
   * @return the sign of the argument.
   * @since 1.5.
   */
  public static double signum(double a)
  {
       if (Double.isNaN(a))
            return Double.NaN;
       if (a > 0)
            return 1.0;
       if (a < 0)
            return -1.0;
       return a;
  }

  /**
   * <p>
   * Returns the sign of the argument as follows:
   * </p>
   * <ul>
   * <li>If <code>a</code> is greater than zero, the result is 1.0f.</li>
   * <li>If <code>a</code> is less than zero, the result is -1.0f.</li>
   * <li>If <code>a</code> is <code>NaN</code>, the result is <code>NaN</code>.
   * <li>If <code>a</code> is positive or negative zero, the result is the
   * same.</li>
   * </ul>
   *
   * @param a the numeric argument.
   * @return the sign of the argument.
   * @since 1.5.
   */
  public static float signum(float a)
  {
       return (float)(signum(a));
  }
}
