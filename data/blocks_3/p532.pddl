;; blocks=5, percentage_new_tower=10, out_folder=., instance_id=532, seed=22

(define (problem blocksworld-532)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 b5 - object)
 (:init
    (clear b4)
    (on b4 b5)
    (on b5 b2)
    (on b2 b3)
    (on-table b3)
    (clear b1)
    (on-table b1))
 (:goal  (and
    (clear b1)
    (on-table b1)
    (clear b4)
    (on b4 b3)
    (on b3 b2)
    (on b2 b5)
    (on-table b5))))
