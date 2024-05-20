;; blocks=5, percentage_new_tower=10, out_folder=., instance_id=518, seed=8

(define (problem blocksworld-518)
 (:domain blocksworld)
(:requirements :typing)
 (:objects b1 b2 b3 b4 b5 - object)
 (:init
    (clear b2)
    (on b2 b5)
    (on b5 b1)
    (on b1 b4)
    (on b4 b3)
    (on-table b3))
 (:goal  (and
    (clear b2)
    (on b2 b1)
    (on-table b1)
    (clear b5)
    (on b5 b4)
    (on b4 b3)
    (on-table b3))))
