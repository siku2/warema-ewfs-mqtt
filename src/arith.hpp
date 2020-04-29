template <typename T>
inline T mod(T a, T b)
{
  const T result = a % b;
  return result >= 0 ? result : result + b;
}

template <typename T>
inline T subModN(T a, T b, T n)
{
  while (b > a)
    a += n;
  return mod<T>(a - b, n);
};
