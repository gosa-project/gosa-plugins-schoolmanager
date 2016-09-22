# Installation

After installation of the .deb package gosa-plugin-schoolmanager, you
need to add the following lines to /etc/gosa/gosa.conf (e.g. below the
ldiftab entry):

```
  <schoolmanagertab>
    <tab class="schoolmanagerintro" name="Introduction" />
    <tab class="importteachers" name="Import Teachers" />
    <tab class="importstudentsandparents" name="Import Students and Parents" />
    <tab class="importstudentsonly" name="Import Students (only)" />
    <tab class="archiveaccounts" name="Archive Accounts" />
  </schoolmanagertab>

```

For the changes to take effect, restart your http daemon (e.g. the Apache web server).
