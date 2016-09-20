# Installation

After installation of the .deb package gosa-plugin-schoolmanager, you
need to add the following lines to /etc/gosa/gosa.conf (e.g. below the
ldiftab entry):

```
  <schoolmanagertab>
    <tab class="schoolmanagerintro" name="Introduction" />
    <tab class="managestudents" name="Manage Students" />
    <tab class="manageteachers" name="Manage Teachers" />
    <tab class="manageparents" name="Manage Parents" />
    <tab class="managecourses" name="Manage Courses" />
    <tab class="archiveaccounts" name="Archive Accounts" />
  </schoolmanagertab>
```

For the changes to take effect, restart your http daemon (e.g. the Apache web server).
