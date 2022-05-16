<input type="hidden" name="phase_10">
<br>
<h3>
    {t 1="11/11"}
        STEP %1: LDAP import completed. Statistical overview of all import actions.
    {/t}
</h3>


<p>
    {if $error == FALSE}
        <b>{t}All entries have been written to the LDAP database successfully.{/t}</b>
    {else}
        <b style="color:red">{t}There has been at least one error during the import of your data.{/t}</b>
    {/if}
</p>

<b>{t}Here is the status report for the import:{/t} </b>

<br>
<br>
FIXME: Todo.
