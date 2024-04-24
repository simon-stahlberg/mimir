; child-snack task with 1 children and 0.0 gluten factor
; constant factor of 1.0
; random seed: 1

(define (problem prob-snack)
  (:domain child-snack)
  (:requirements :typing :equality)
  (:objects
    child1 - child
    bread1 - bread-portion
    content1 - content-portion
    tray1 - tray
    table1 - place
    sandw1 - sandwich
  )
  (:init
     (at tray1 kitchen)
     (at_kitchen_bread bread1)
     (at_kitchen_content content1)
     (not_allergic_gluten child1)
     (waiting child1 table1)
     (notexist sandw1)
  )
  (:goal
    (and
     (served child1)
    )
  )
)
