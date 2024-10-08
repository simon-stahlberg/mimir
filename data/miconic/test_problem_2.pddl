(define (problem mixed-f2-p2-u0-v0-d0-a0-n0-A0-B0-N0-F0)
   (:domain miconic)
   (:requirements :strips :typing)
   (:objects p0 - passenger
             f0 f1 - floor)

(:init
(above f0 f1)
(origin p0 f0)
(destin p0 f1)
(lift-at f0)
)

(:goal
(and
(served p0)
))
)
