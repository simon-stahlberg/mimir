;; blocks=5, percentage_new_tower=0, out_folder=., instance_id=499, seed=19

(define (problem blocksworld-499)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 b5 - object)
 (:init
    (clear b5)
    (on b5 b1)
    (on b1 b4)
    (on b4 b2)
    (on b2 b3)
    (on-table b3))
 (:goal  (and
    (clear b1)
    (on b1 b2)
    (on b2 b3)
    (on b3 b4)
    (on b4 b5)
    (on-table b5))))
