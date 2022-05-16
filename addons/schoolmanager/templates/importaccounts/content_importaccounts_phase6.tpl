<input type="hidden" name="phase_06">
<br>
<h3>
    {t 1="6/11"}
        STEP %1  (LDAP import status): 
    {/t}
</h3>

{if count($data_groups) > 0}
    <br>
    <b>{t}Standard (POSIX) groups{/t}:</b></p>
    <hr>
    <br>

    {foreach from=$data_groups item=group key=key}
    <div style="float:left; width:24em; height:30ex;">
    <table summary="{t 1=$group['cn'][0]}Group object: %1{/t}" cellspacing="1" border=0 cellpadding="4" bgcolor="#FEFEFE">
            <tr>
            <td bgcolor="#BBBBBB" colspan="2">
                    {t 1=$key}Group object: %1{/t}
            </td>
            <td style="width:1em;">&nbsp;</td>
            </tr>
            {foreach from=$group item=value key=property}
            <tr>
                    <td bgcolor="#EEEEEE">
                    <b>{$property}:</b>
                    </td>
                    <td bgcolor="#FEFEFE">
                    {$group[$property][0]}
                    </td>
                    <td style="width:1em;">&nbsp;</td>
            </tr>
            {/foreach}
    </table>
    <br>
    </div>
    {/foreach}
    <div class="clear"></div>

    <br>
    <br>
{/if}

{if count($data_ogroups) > 0}
    <b>{t}Object groups{/t}:</b>
    <hr>
    <br>

    {foreach from=$data_ogroups item=group key=key}
    <div style="float:left; width:24em; height:30ex;">
    <table summary="{t 1=$group['cn'][0]}Group object: %1{/t}" cellspacing="1" border=0 cellpadding="4" bgcolor="#FEFEFE">
            <tr>
            <td bgcolor="#BBBBBB" colspan="2">
                    {t 1=$key}Group object: %1{/t}
            </td>
            <td style="width:1em;">&nbsp;</td>
            </tr>
    {foreach from=$group item=value key=property}
    {if $property!=="member"}
            <tr>
                    <td bgcolor="#EEEEEE">
                    <b>{$property}:</b>
                    </td>
                    <td bgcolor="#FEFEFE">
                    {$group[$property][0]}
                    </td>
                    <td style="width:1em;">&nbsp;</td>
            </tr>
    {/if}
    {/foreach}
    </table>
    <br>
    </div>
    {/foreach}
{/if}
<div class="clear"></div>
