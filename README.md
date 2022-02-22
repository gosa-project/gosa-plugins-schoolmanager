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

## Debug GOsa² schoolmanager plugin using Xdebug

1. Install the `php-xdebug` package on your GOsa² server by executing
the following command:

Debian (9+): `sudo apt install php-xdebug`

This will automagically configure apache2 too.

More information: https://xdebug.org/docs/install

2. Configure Xdebug's .ini file to use your local IDE:

`sudo editor /etc/php/7.4/mods-available/xdebug.ini` (Debian)
```ini
zend_extension=xdebug.so

xdebug.profiler_enable_trigger = 1
xdebug.profiler_enable = 0
xdebug.profiler_out_dir = "/tmp"

xdebug.remote_enable = 1
xdebug.start_with_request = yes

xdebug.idekey = <REPLACE ME WITH IDE KEY ('VSCODE', 'PHPSTORM'…)>
xdebug.mode = debug

xdebug.client_host = "<REPLACE ME WITH DEV HOST IP>"
xdebug.client_port = 9003

xdebug.log = "/tmp/xdebug.log"
```

**WARNING:** Many Linux distributions now use systemd, which implements
private tmp directories. This means that when PHP is run through a web
server or as PHP-FPM, the /tmp directory is prefixed with something akin to:

`/tmp/systemd-private-ea3cfa882b4e478993e1994033fc5feb-apache.service-FfWZRg`


You can also set up a port forwarding tunnel using SSH,
if you're using two different machines (or a VM) to develop:

`ssh user@remote-machine -R 9003:127.0.0.1:9003`

The Xdebug server connects to the IDE client!
So the IDE client needs to **listen** at port `9003` in this example.

The IDE key depends on the IDE you're using. Visual Studio Code uses
`VSCODE` and PhpStorm uses `PHPSTORM`. Most IDE's let you configure this though.

3. Test your installation.

First restart apache2.

`sudo systemctl restart apache2`


Execute `php -v`, it should look like this:
```bash
PHP 7.4.25 (cli) (built: Oct 23 2021 21:53:50) ( NTS )
Copyright (c) The PHP Group
Zend Engine v3.4.0, Copyright (c) Zend Technologies
    with Zend OPcache v7.4.25, Copyright (c), by Zend Technologies
    with Xdebug v3.0.2, Copyright (c) 2002-2021, by Derick Rethans
```

4. Install xdebug-helper extension on your development browser.

Your internet browser needs to sent a specific header to the server to
indicate a debugging session.
This can be done easily using an extension.

[xdebug-helper Firefox](https://github.com/BrianGilbert/xdebug-helper-for-firefox)

[xdebug-helper Chrome](https://github.com/BrianGilbert/xdebug-helper-for-chrome)

5. Configure your IDE (In this example VS Code)

* Install `PHP Debug` extension: https://marketplace.visualstudio.com/items?itemName=xdebug.php-debug

* Configure VS Code to properly launch a PHP debug session.
Example `.vscode/launch.json`:

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Listen for Xdebug",
            "type": "php",
            "request": "launch",
            "port": 9003,
            // server -> local
            "pathMappings": {
                "/usr/share/gosa/plugins/": "${workspaceRoot}",
            },
            "postDebugTask": "sleep-task",
            "log": true,
        },
    ]
}
```

The `pathMappings` setting is VERY important and helpful!
Imagine you work on `~/dev/test.php` and your apache2
is configured to use `/var/www/html/(test.php)`. Your IDE will report any breakpoints
using path `~/dev/test.php`. But the Xdebug server won't
find any files under `~/dev/test.php` since it's a different machine!

`pathMappings` maps `~/dev` to `/var/www/html` and vice-versa.

Also you have to keep your local files and the server files in sync!
This can be archived by using a SSHFS mount to develop directly on the
server's files.

`sshfs <local-mount-dir> root@<remote-server-ip>:/var/www/html`

Also the xdebug client inside of VS Code can sometimes be slow to restart,
so wait a bit after closing this debug session:

`.vscode/tasks.json`:
```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "sleep-task",
            "command": "sleep 0.5s",
            "type": "shell",
            "presentation": {
                "reveal": "never"
            },
        }
    ]
}
```
