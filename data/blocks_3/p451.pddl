;; blocks=4, percentage_new_tower=40, out_folder=., instance_id=451, seed=1

(define (problem blocksworld-451)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 - object)
 (:init
    (clear b3)
    (on b3 b2)
    (on b2 b1)
    (on-table b1)
    (clear b4)
    (on-table b4))
 (:goal  (and
    (clear b1)
    (on b1 b3)
    (on-table b3)
    (clear b2)
    (on-table b2)
    (clear b4)
    (on-table b4))))
