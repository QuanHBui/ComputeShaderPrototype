#pragma once

#ifndef P3_EPA_H
#define P3_EPA_H

class P3Collider;
class P3Simplex;

/**
 * Reference: https://blog.winter.dev/2020/epa-algorithm/
 */
void P3Epa(P3Collider const &, P3Collider const &, P3Simplex &);

#endif // P3_EPA_H
