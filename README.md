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

## Data protection and Performance Amendment Recommendations

```diff
diff --git a/ldap/slapd-debian-edu.conf b/ldap/slapd-debian-edu.conf
index 52ad2a8..33340aa 100644
--- a/ldap/slapd-debian-edu.conf
+++ b/ldap/slapd-debian-edu.conf
@@ -89,9 +89,11 @@ index                givenname       eq
 index          displayName     eq
 #index         telephoneNumber eq
 #attributes that pop up in syslog when using schoolmanager
+index           dateOfBirth,gender     eq
 index           member          eq
 index           roleOccupant    eq
 index           manager         eq
+index           mail,alias      pres,eq,sub
 
 #samba index
 index sambaSID                          eq
@@ -100,6 +102,9 @@ index sambaDomainName                   eq
 index sambaGroupType                    eq
 index sambaSIDList                      eq
 
+#GOsa²
+index gosaObject                        eq
+
 # PowerDNS index
 index associatedDomain         pres,eq,sub
 index aRecord                      pres,eq
@@ -139,7 +144,7 @@ access to *
         by group.exact="cn=ldap-admins,ou=ldap-access,dc=skole,dc=skolelinux,dc=no" manage
         by * none break
 
-access to attrs=userPassword,gender,birthOfdate
+access to attrs=userPassword,gender,dateOfBirth
        by self      =wx
        by anonymous auth
        by set="[cn=admins,ou=group,dc=skole,dc=skolelinux,dc=no]/member & this" none
```
