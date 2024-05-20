;; blocks=3, percentage_new_tower=0, out_folder=., instance_id=260, seed=20

(define (problem blocksworld-260)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 - object)
 (:init
    (clear b1)
    (on b1 b2)
    (on b2 b3)
    (on-table b3))
 (:goal  (and
    (clear b3)
    (on b3 b2)
    (on b2 b1)
    (on-table b1))))
