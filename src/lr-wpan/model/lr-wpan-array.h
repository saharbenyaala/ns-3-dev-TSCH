/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 NXP
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author:
 *  Alfred Park <park@gtech-systems.com>
 *  Peishuo Li <pressthunder@gmail.com>
 */
#ifndef LR_WPAN_ARRAY_H
#define LR_WPAN_ARRAY_H

#include <vector>
#include <ns3/integer.h>

/**
 * \ingroup lr-wpan
 *
 * 2D array to store data needed in lr-wpan model
 *
 */
namespace ns3 {
  template <typename T>
  class LrWpanArray2D
  {
    public:
      LrWpanArray2D (u_int32_t x, u_int32_t y) : p (new T*[x]), m_xMax (x), m_yMax (y)
        {
          for (u_int32_t i = 0; i < m_xMax; i++)
            p[i] = new T[y];
        }

      ~LrWpanArray2D (void)
        {
          for (u_int32_t i = 0; i < m_xMax; i++)
            delete[] p[i];
          delete p;
          p = 0;
        }

      T* operator[] (const u_int32_t i) const
        {
           return p[i];
        }
    private:
      T** p;
      u_int32_t m_xMax;
      u_int32_t m_yMax;
  };

/**
* \ingroup lr-wpan
*
* 3D array to store data needed in lr-wpan model
*
*/
  template <typename T>
  class LrWpanArray3D
  {
    public:
      LrWpanArray3D()
      {
         p = 0;
         m_xMax = 0;
         m_yMax = 0;
         m_zMax = 0;
      }

      void Build (u_int32_t x, u_int32_t y, u_int32_t z)
        {
          p = new LrWpanArray2D<T>*[x];
          m_xMax = x;
          m_yMax = y;
          m_zMax = z;
          for (u_int32_t i = 0; i < m_xMax; i++)
            p[i] = new LrWpanArray2D<T> (y, z);
        }

      ~LrWpanArray3D (void)
        {
          for (u_int32_t i = 0; i < m_xMax; i++)
            {
              delete p[i];
              p[i] = 0;
            }
          delete[] p;
          p = 0;
        }

      int GetSize(void)
      {
        return m_xMax*m_yMax*m_zMax;
      }

      LrWpanArray2D<T>& operator[] (const u_int32_t i) const
        {
          return *(p[i]);
        }
    private:
      LrWpanArray2D<T>** p;
      u_int32_t m_xMax;
      u_int32_t m_yMax;
      u_int32_t m_zMax;
  };

} // namespace ns3

#endif /* LR_WPAN_ARRAY_H */
