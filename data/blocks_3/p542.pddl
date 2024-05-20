;; blocks=5, percentage_new_tower=20, out_folder=., instance_id=542, seed=2

(define (problem blocksworld-542)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 b5 - object)
 (:init
    (clear b1)
    (on b1 b2)
    (on b2 b5)
    (on b5 b3)
    (on b3 b4)
    (on-table b4))
 (:goal  (and
    (clear b4)
    (on b4 b1)
    (on b1 b3)
    (on b3 b2)
    (on b2 b5)
    (on-table b5))))
