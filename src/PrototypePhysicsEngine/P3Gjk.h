#pragma once

#ifndef P3_GJK_H
#define P3_GJK_H

class P3Collider;
class P3Simplex;

bool gjk(P3Collider const&, P3Collider const&, class P3Simplex&);

#endif // P3_GJK_H