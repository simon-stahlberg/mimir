(define (problem num_passengers_1)
   (:domain miconic)
   (:requirements :adl)
   (:objects
      p0 - passenger
      f0 f1 - floor)
   (:init
      (above f0 f1)
      (origin p0 f1)
      (destin p0 f0)
      (lift-at f0))
   (:goal (and (served p0)))
)


