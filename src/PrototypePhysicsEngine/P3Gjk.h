#pragma once

#ifndef P3_GJK_H
#define P3_GJK_H

class P3Collider;
class P3Simplex;

/**
 * Reference: https://blog.winter.dev/2020/gjk-algorithm/
 */
bool P3Gjk(P3Collider const &, P3Collider const &, P3Simplex &);

#endif // P3_GJK_H
