# School Manager Add-On for GOsa²

This GOsa² Add-On has been especially designed for user and group management
at schools. In our experience, schools have several special needs uncommon to
other IT setups, e.g.:

  * many users come and go every year
  * group memberships may change every half year
  * each student comes with parents that are also involved in
    school IT (e.g., home page login)

With GOsa² School Manager you can mass create user accounts for teachers,
students and parents via importing two (large) CSV files.

From the CSV file, various information gets extracted:

  * Personal data (name, date of birth, unique ID as stored in the school
    administration software)
  * Teachers: class teacher, taught subjects, taught courses
  * Students: each student normally comes with one or two parents that
    also may need accounts in LDAP
  * Students and teachers are grouped by classes and courses
  * Students and their parents are bound together via parent groups
  * Mail addresses of teachers, students and parents.

From the set of information stored, various LDAP client applications can be
fed with:

  * e-Learning system (classes and courses are already stored in LDAP)
  * e-Mail system with mailing lists
  * group based collaboration on the file system level
  * etc.

The School Manager Plugin for GOsa² has been licensed under GPL-2+. For further details
see the AUTHORS.txt and COPYING files.
