// ======================================================================== //
// Copyright 2009-2016 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#pragma once

#include "grid_soa.h"
#include "../common/ray.h"
#include "triangle_intersector_pluecker.h"

namespace embree
{
  namespace isa
  {
    class GridSOAIntersector1
    {
    public:
      typedef SubdivPatch1Cached Primitive;
      
      class Precalculations 
      { 
      public:
        __forceinline Precalculations (Ray& ray, const void* ptr) 
          : grid(nullptr) {}
        
        __forceinline ~Precalculations() 
        {
	  if (grid)
            SharedLazyTessellationCache::sharedLazyTessellationCache.unlock();
        }
        
      public:
        GridSOA* grid;
      };
      
      template<typename Loader>
        static __forceinline void intersect(Ray& ray,
                                            const RTCIntersectContext* context, 
                                            const float* const grid_x,
                                            const size_t line_offset,
                                            Precalculations& pre,
                                            Scene* scene)
      {
        typedef typename Loader::vfloat vfloat;
        const size_t dim_offset    = pre.grid->dim_offset;
        const float* const grid_y  = grid_x + 1 * dim_offset;
        const float* const grid_z  = grid_x + 2 * dim_offset;
        const float* const grid_uv = grid_x + 3 * dim_offset;

        Vec3<vfloat> v0, v1, v2;
        Loader::gather(grid_x,grid_y,grid_z,line_offset,v0,v1,v2);
       
        auto mapUV = [&](vfloat& u, vfloat& v) {
          const Vec3<vfloat> tri_v012_uv = Loader::gather(grid_uv,line_offset);	
          const Vec2<vfloat> uv0 = GridSOA::decodeUV(tri_v012_uv[0]);
          const Vec2<vfloat> uv1 = GridSOA::decodeUV(tri_v012_uv[1]);
          const Vec2<vfloat> uv2 = GridSOA::decodeUV(tri_v012_uv[2]);        
          const Vec2<vfloat> uv = u * uv1 + v * uv2 + (1.0f-u-v) * uv0;        
          u = uv[0];v = uv[1]; 
        };

        PlueckerIntersector1<Loader::M> intersector(ray,nullptr);
        intersector.intersect(ray,v0,v1,v2,mapUV,Intersect1EpilogMU<Loader::M,true>(ray,context,pre.grid->geomID,pre.grid->primID,scene,nullptr));
      };
      
      template<typename Loader>
        static __forceinline bool occluded(Ray& ray,
                                           const RTCIntersectContext* context, 
                                           const float* const grid_x,
                                           const size_t line_offset,
                                           Precalculations& pre,
                                           Scene* scene)
      {
        typedef typename Loader::vfloat vfloat;
        const size_t dim_offset    = pre.grid->dim_offset;
        const float* const grid_y  = grid_x + 1 * dim_offset;
        const float* const grid_z  = grid_x + 2 * dim_offset;
        const float* const grid_uv = grid_x + 3 * dim_offset;

        Vec3<vfloat> v0, v1, v2;
        Loader::gather(grid_x,grid_y,grid_z,line_offset,v0,v1,v2);
        
        auto mapUV = [&](vfloat& u, vfloat& v) {
          const Vec3<vfloat> tri_v012_uv = Loader::gather(grid_uv,line_offset);	
          const Vec2<vfloat> uv0 = GridSOA::decodeUV(tri_v012_uv[0]);
          const Vec2<vfloat> uv1 = GridSOA::decodeUV(tri_v012_uv[1]);
          const Vec2<vfloat> uv2 = GridSOA::decodeUV(tri_v012_uv[2]);        
          const Vec2<vfloat> uv = u * uv1 + v * uv2 + (1.0f-u-v) * uv0;        
          u = uv[0];v = uv[1]; 
        };

        PlueckerIntersector1<Loader::M> intersector(ray,nullptr);
        return intersector.intersect(ray,v0,v1,v2,mapUV,Occluded1EpilogMU<Loader::M,true>(ray,context,pre.grid->geomID,pre.grid->primID,scene,nullptr));
      }
      
      /*! Intersect a ray with the primitive. */
      static __forceinline void intersect(Precalculations& pre, Ray& ray, const RTCIntersectContext* context, const Primitive* prim, size_t ty, Scene* scene, size_t& lazy_node) 
      {
        const size_t line_offset   = pre.grid->width;
        const float* const grid_x  = pre.grid->decodeLeaf(0,prim);
        
#if defined(__AVX__)
        intersect<GridSOA::Gather3x3>( ray, context, grid_x, line_offset, pre, scene);
#else
        intersect<GridSOA::Gather2x3>(ray, context, grid_x            , line_offset, pre, scene);
        intersect<GridSOA::Gather2x3>(ray, context, grid_x+line_offset, line_offset, pre, scene);
#endif
      }
      
      /*! Test if the ray is occluded by the primitive */
      static __forceinline bool occluded(Precalculations& pre, Ray& ray, const RTCIntersectContext* context, const Primitive* prim, size_t ty, Scene* scene, size_t& lazy_node) 
      {
        const size_t line_offset   = pre.grid->width;
        const float* const grid_x  = pre.grid->decodeLeaf(0,prim);
        
#if defined(__AVX__)
        return occluded<GridSOA::Gather3x3>( ray, context, grid_x, line_offset, pre, scene);
#else
        if (occluded<GridSOA::Gather2x3>(ray, context, grid_x            , line_offset, pre, scene)) return true;
        if (occluded<GridSOA::Gather2x3>(ray, context, grid_x+line_offset, line_offset, pre, scene)) return true;
#endif
        return false;
      }      
    };
  }
}
