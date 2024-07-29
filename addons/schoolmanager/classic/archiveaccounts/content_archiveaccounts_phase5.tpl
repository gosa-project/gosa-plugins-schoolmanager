<input type="hidden" name="phase_05">

<br><h3>{t 1="6/11"}STEP %1: Confirm archived user accounts{/t}</h3>

{foreach from=$data item=row key=key}
    <table summary="{t 1=$data[$key]['main_account']['sn'][0] 2=$data[$key]['main_account']['givenName'][0]}User account: %1, %2{/t}" cellspacing="1" border=0 cellpadding="4" bgcolor="#FEFEFE">
        <tr>
    {if isset($data[$key]['aux_accounts'])}
            <td bgcolor="#BBBBBB" colspan={2 + count($data[$key]['aux_accounts'])}>
    {else}
            <td bgcolor="#BBBBBB" colspan="2">
    {/if}
    {t 1=$data[$key]['main_account']['sn'][0] 2=$data[$key]['main_account']['givenName'][0]}User account: %1, %2{/t}
            </td>
        </tr>
    {if isset($data[$key]['aux_accounts'])}
        <tr>
            <td>
                <table summary="{t 1=$data[$key]['main_account']['sn'][0] 2=$data[$key]['main_account']['givenName'][0]}Account group for %1, %2{/t}">
    {/if}
    {foreach from=$data[$key]['main_account'] item=value key=property}
                    <tr>
                        <td bgcolor="#EEEEEE">
                            <b>{$property}:</b>
                        </td>
                        <td bgcolor="#F8F8F8">
        {if $property != "userPassword" || $data[$key]['main_account'][$property][0]===""}
                                {$data[$key]['main_account'][$property][0]}
        {elseif strpos($data[$key]['main_account']['_status'][0],"exists")!==FALSE }
                            {t}<keep>{/t}
        {elseif strpos($data[$key]['main_account']['_status'][0],"data-incomplete")!==FALSE }
        {else}
                            ************
        {/if}
                        </td>
                    </tr>
    {/foreach}
    </table>
    <br>
{/foreach}
