<input type="hidden" name="phase_05">
<h3>
    {t 1="5/10"}
        STEP %1  (LDAP import status): 
    {/t}
</h3>

<div class="row">
{if count($data_groups) > 0}
    <h4>{t}Standard (POSIX) groups{/t}</h4>
    <hr class="divider">

    {foreach from=$data_groups item=group key=key}
    <div class="col s6" style="padding:.75em;">
    <table summary="{t 1=$group['cn'][0]}Group object: %1{/t}">
        <thead>
            <tr>
            <th colspan="2">
                    {t 1=$key}Group object: %1{/t}
            </th>
            </tr>
        </thead>
        <tbody>
            {foreach from=$group item=value key=property}
            <tr>
                    <td>
                    <b>{$property}:</b>
                    </td>
                    <td>
                    {$group[$property][0]}
                    </td>
            </tr>
            {/foreach}
        </tbody>
    </table>
    </div>
    {/foreach}
{/if}
</div>

<div class="row">
{if count($data_ogroups) > 0}
    <h4>{t}Object groups{/t}</h4>
    <hr class="divider">

    {foreach from=$data_ogroups item=group key=key}
    <div class="col s6" style="padding:.75em;">
    <table summary="{t 1=$group['cn'][0]}Group object: %1{/t}">
        <thead>
            <tr>
            <th colspan="2">
                    {t 1=$key}Group object: %1{/t}
            </th>
            </tr>
        </thead>
        <tbody>
    {foreach from=$group item=value key=property}
    {if $property!=="member"}
            <tr>
                    <td>
                    <b>{$property}:</b>
                    </td>
                    <td>
                    {$group[$property][0]}
                    </td>
            </tr>
    {/if}
    {/foreach}
        </tbody>
    </table>
    </div>
    {/foreach}
{/if}
</div>
