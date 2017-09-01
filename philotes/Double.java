/* Double.java -- Philotes Double wrapper
   Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005
   Free Software Foundation, Inc.

Adapted from GNU Classpath.  Philotes and this file GPLv3.

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

/**
 * Instances of class <code>Double</code> represent primitive
 * <code>double</code> values.
 *
 * Additionally, this class provides various helper functions and variables
 * related to doubles.
 *
 * @author Paul Fisher
 * @author Andrew Haley (aph@cygnus.com)
 * @author Eric Blake (ebb9@email.byu.edu)
 * @author Tom Tromey (tromey@redhat.com)
 * @author Andrew John Hughes (gnu_andrew@member.fsf.org)
 * @since 1.0
 * @status partly updated to 1.5
 */
public class Double
{
  /**
   * Compatible with JDK 1.0+.
   */
  private static final long serialVersionUID = -9172774392245257468;

  /**
   * The maximum positive value a <code>double</code> may represent
   * is 1.7976931348623157e+308.
   */
  public static final double MAX_VALUE = 1.7976931348623157e+308;

  /**
   * The minimum positive value a <code>double</code> may represent
   * is 5e-324.
   */
  public static final double MIN_VALUE = 5e-324;

  /**
   * The value of a double representation -1.0/0.0, negative
   * infinity.
   */
  public static final double NEGATIVE_INFINITY = -1.0 / 0.0;

  /**
   * The value of a double representing 1.0/0.0, positive infinity.
   */
  public static final double POSITIVE_INFINITY = 1.0 / 0.0;

  /**
   * All IEEE 754 values of NaN have the same value in Java.
   */
  public static final double NaN = 0.0 / 0.0;

  /**
   * The number of bits needed to represent a <code>double</code>.
   * @since 1.5
   */
  public static final int SIZE = 64;

  /**
   * Cache representation of 0
   */
  private static final Double ZERO = new Double(0.0);

  /**
   * Cache representation of 1
   */
  private static final Double ONE = new Double(1.0);

  /**
   * The immutable value of this Double.
   *
   * @serial the wrapped double
   */
  private final double value;

  public static double longBitsToDouble(long a)
  {
    #prayer
  }

  public static long doubleToLongBits(double a)
  {
    #prayer
  }

  /**
   * Create a <code>Double</code> from the primitive <code>double</code>
   * specified.
   *
   * @param value the <code>double</code> argument
   */
  public Double(double value_)
  {
    value = value_;
  }

  /**
   * Convert the <code>double</code> to a <code>String</code>.
   * Floating-point string representation is fairly complex: here is a
   * rundown of the possible values.  "<code>[-]</code>" indicates that a
   * negative sign will be printed if the value (or exponent) is negative.
   * "<code>&lt;number&gt;</code>" means a string of digits ('0' to '9').
   * "<code>&lt;digit&gt;</code>" means a single digit ('0' to '9').<br>
   *
   * <table border=1>
   * <tr><th>Value of Double</th><th>String Representation</th></tr>
   * <tr><td>[+-] 0</td> <td><code>[-]0.0</code></td></tr>
   * <tr><td>Between [+-] 10<sup>-3</sup> and 10<sup>7</sup>, exclusive</td>
   *     <td><code>[-]number.number</code></td></tr>
   * <tr><td>Other numeric value</td>
   *     <td><code>[-]&lt;digit&gt;.&lt;number&gt;
   *          E[-]&lt;number&gt;</code></td></tr>
   * <tr><td>[+-] infinity</td> <td><code>[-]Infinity</code></td></tr>
   * <tr><td>NaN</td> <td><code>NaN</code></td></tr>
   * </table>
   *
   * Yes, negative zero <em>is</em> a possible value.  Note that there is
   * <em>always</em> a <code>.</code> and at least one digit printed after
   * it: even if the number is 3, it will be printed as <code>3.0</code>.
   * After the ".", all digits will be printed except trailing zeros. The
   * result is rounded to the shortest decimal number which will parse back
   * to the same double.
   *
   * <p>To create other output formats, use {@link java.text.NumberFormat}.
   *
   * @XXX specify where we are not in accord with the spec.
   *
   * @param d the <code>double</code> to convert
   * @return the <code>String</code> representing the <code>double</code>
   */
  public static String toString(double d)
  {
    #prayer
  }

  /**
   * Returns a <code>Double</code> object wrapping the value.
   * In contrast to the <code>Double</code> constructor, this method
   * may cache some values.  It is used by boxing conversion.
   *
   * @param val the value to wrap
   * @return the <code>Double</code>
   * @since 1.5
   */
  public static Double valueOf(double val)
  {
    if (val == 0.0)
      return ZERO;
    else if (val == 1.0)
      return ONE;
    else
      return new Double(val);
  }

  /**
   * Return <code>true</code> if the <code>double</code> has the same
   * value as <code>NaN</code>, otherwise return <code>false</code>.
   *
   * @param v the <code>double</code> to compare
   * @return whether the argument is <code>NaN</code>.
   */
  public static boolean isNaN(double v)
  {
    // This works since NaN != NaN is the only reflexive inequality
    // comparison which returns true.
    return v != v;
  }

  /**
   * Return <code>true</code> if the <code>double</code> has a value
   * equal to either <code>NEGATIVE_INFINITY</code> or
   * <code>POSITIVE_INFINITY</code>, otherwise return <code>false</code>.
   *
   * @param v the <code>double</code> to compare
   * @return whether the argument is (-/+) infinity.
   */
  public static boolean isInfinite(double v)
  {
    return v == POSITIVE_INFINITY || v == NEGATIVE_INFINITY;
  }

  /**
   * Return <code>true</code> if the value of this <code>Double</code>
   * is the same as <code>NaN</code>, otherwise return <code>false</code>.
   *
   * @return whether this <code>Double</code> is <code>NaN</code>
   */
  public boolean isNaN()
  {
    return isNaN(value);
  }

  /**
   * Return <code>true</code> if the value of this <code>Double</code>
   * is the same as <code>NEGATIVE_INFINITY</code> or
   * <code>POSITIVE_INFINITY</code>, otherwise return <code>false</code>.
   *
   * @return whether this <code>Double</code> is (-/+) infinity
   */
  public boolean isInfinite()
  {
    return isInfinite(value);
  }

  /**
   * Return the value of this <code>Double</code> as a <code>byte</code>.
   *
   * @return the byte value
   * @since 1.1
   */
  public byte byteValue()
  {
    return (byte) value;
  }

  /**
   * Return the value of this <code>Double</code> as a <code>short</code>.
   *
   * @return the short value
   * @since 1.1
   */
  public short shortValue()
  {
    return (short) value;
  }

  /**
   * Return the value of this <code>Double</code> as an <code>int</code>.
   *
   * @return the int value
   */
  public int intValue()
  {
    return (int) value;
  }

  /**
   * Return the value of this <code>Double</code> as a <code>long</code>.
   *
   * @return the long value
   */
  public long longValue()
  {
    return (long) value;
  }

  /**
   * Return the value of this <code>Double</code> as a <code>float</code>.
   *
   * @return the float value
   */
  public float floatValue()
  {
    return (float) value;
  }

  /**
   * Return the value of this <code>Double</code>.
   *
   * @return the double value
   */
  public double doubleValue()
  {
    return value;
  }
}
